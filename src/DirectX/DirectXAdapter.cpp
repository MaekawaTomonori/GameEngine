#include "DirectXAdapter.hpp"

#include <cassert>
#include <thread>

#include "include/Log.hpp"
#include "include/Utils.hpp"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

FrameRateLimiter::FrameRateLimiter(uint16_t maxFps, bool useVsync): maxFps_(maxFps), vsyncEnabled_(useVsync) {
	reference_ = std::chrono::steady_clock::now();
}

void FrameRateLimiter::WaitForNextFrame() {

	const std::chrono::microseconds TargetFrameTime(static_cast<uint64_t>(1e3 / (maxFps_ + 5)));

	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::chrono::microseconds elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	if (elapsedTime < TargetFrameTime){
		std::chrono::microseconds remaining = TargetFrameTime - elapsedTime;
		while (std::chrono::milliseconds(2) < remaining) {
			std::this_thread::sleep_for(remaining - std::chrono::milliseconds(2));
		}

		while (std::chrono::steady_clock::now() - reference_ < TargetFrameTime) {
			std::this_thread::yield();
		}
	}

	reference_ = std::chrono::steady_clock::now();
}

DirectXAdapter::DirectXAdapter(const HWND _hWnd, size_t _width, size_t _height) :windowSize_(_width, _height), hWnd_(_hWnd) {
	EnableDebugLayer();
	if (!CreateDXGI())Utils::Alert("Failed to create DXGI");
	if (!InfoQueue()) Utils::Alert("Failed to create InfoQueue");
	if (!CreateCommand())Utils::Alert("Failed to create Command");
	if (!CreateSwapChain()) Utils::Alert("Failed to create SwapChain");
	if (!CreateRTV()) Utils::Alert("Failed to create RTV");
	if (!CreateFence()) Utils::Alert("Failed to create Fence");
	Log::Send(Log::Level::INFO, "DirectXAdapter Initialized");
}

void DirectXAdapter::Register(std::function<void()> _task) {
	if (_task){
		renderingCommands_.emplace_back(std::move(_task));
	} else{
		Log::Send(Log::Level::ERR, "Rendering command is null");
	}
}

void DirectXAdapter::Render() {
	//Log::Send(Log::Level::INFO, "Render Start");

	UINT bbi = swapChain_->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChainResources_[bbi].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	cList_->ResourceBarrier(1, &barrier);



	cList_->OMSetRenderTargets(1, &rtvHandles_[bbi], false, nullptr);
	cList_->ClearRenderTargetView(rtvHandles_[bbi], &back.x, 0, nullptr);


	// Execute rendering commands
	for (const auto& command : renderingCommands_){
		if (command){
			command();
		}
	}
	renderingCommands_.clear();

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	cList_->ResourceBarrier(1, &barrier);

	if (FAILED(cList_->Close())) {
		Utils::Alert("Failed to close command list");
		return;
	}
	ID3D12CommandList* commandLists[] = {cList_.Get()};
	cQueue_->ExecuteCommandLists(_countof(commandLists), commandLists);
	swapChain_->Present(1, 0);

	Wait();

	if (FAILED(cAllocator_->Reset())){
		Utils::Alert("Failed to reset command allocator");
		return;
	}

	if (FAILED(cList_->Reset(cAllocator_.Get(), nullptr))) {
		Utils::Alert("Failed to reset command list");
		return;
	}

}

void DirectXAdapter::EnableDebugLayer() {
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer_)))){
		debugLayer_->EnableDebugLayer();
		debugLayer_->SetEnableGPUBasedValidation(true);
		debugLayer_->SetEnableSynchronizedCommandQueueValidation(true);
		Log::Send(Log::Level::INFO, "Debug Layer Enabled");
	} else{
		Log::Send(Log::Level::ERR, "Failed to enable debug layer");
	}
}

bool DirectXAdapter::CreateDXGI() {
	Log::Send(Log::Level::INFO, "Create DXGI");

	//Factory
	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory_)))){
		return false;
	}

	Log::Send(Log::Level::INFO, "Factory Created");

	//Adaptor
	for (UINT i = 0; SUCCEEDED(factory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter_))); ++i){
		// Check if the adapter supports Direct3D 12
		DXGI_ADAPTER_DESC3 desc;
		if (FAILED(adapter_->GetDesc3(&desc))){
			adapter_ = nullptr;
		}
		if (!(desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)){
			break;
		}
		adapter_ = nullptr;
	}

	if (adapter_ == nullptr)return false;

	Log::Send(Log::Level::INFO, "Adapter Created");

	//Device
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelNames[] = {
		"12.2",
		"12.1",
		"12.0"
	};

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		if (SUCCEEDED(D3D12CreateDevice(adapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_)))){
			Log::Send(Log::Level::INFO, "Device Created");
			Log::Send(Log::Level::INFO, "Feature Level: " + std::string(featureLevelNames[i]));
			break;
		}

		Log::Send(Log::Level::ERR, "Failed to create device with feature level: " + std::string(featureLevelNames[i]));
	}

	if (device_ == nullptr){
		Log::Send(Log::Level::ERR, "Failed to create device");
		return false;
	}

	Log::Send(Log::Level::INFO, "Complete create DXGI");
	return true;
}

bool DirectXAdapter::InfoQueue() const {
#ifdef _DEBUG
	{
		Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
		if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))){
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

			D3D12_MESSAGE_ID denyIds[] = {D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE};
			D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(denyIds);
			filter.DenyList.pIDList = denyIds;
			filter.DenyList.NumSeverities = _countof(severities);
			filter.DenyList.pSeverityList = severities;
			infoQueue->PushStorageFilter(&filter);
		}
	}
#endif
	return true;
}

bool DirectXAdapter::CreateCommand() {
	//Queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	if (FAILED(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cQueue_)))) {
		Log::Send(Log::Level::ERR, "Failed to create command queue");
		return false;
	}
	Log::Send(Log::Level::INFO, "Command Queue Created");

	//Allocator
	if (FAILED(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cAllocator_)))){
		Log::Send(Log::Level::ERR, "Failed to create command allocator");
		return false;
	}
	Log::Send(Log::Level::INFO, "Command Allocator Created");

	//List
	if (FAILED(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cAllocator_.Get(), nullptr, IID_PPV_ARGS(&cList_)))){
		Log::Send(Log::Level::ERR, "Failed to create command list");
		return false;
	}
	Log::Send(Log::Level::INFO, "Command List Created");

	Log::Send(Log::Level::INFO, "Complete create Commands");
	return true;
}

bool DirectXAdapter::CreateSwapChain() {
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.Width = static_cast<UINT>(windowSize_.first);
	desc.Height = static_cast<UINT>(windowSize_.second);
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	if (FAILED(factory_->CreateSwapChainForHwnd(cQueue_.Get(), hWnd_, &desc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf())))){
		Log::Send(Log::Level::ERR, "Failed to create swap chain");
		return false;
	}

	Log::Send(Log::Level::INFO, "Swap Chain Created");

	return true;
}

bool DirectXAdapter::CreateRTV() {
	rtvHeap_ = std::make_unique<Heap>();
	if (!rtvHeap_ || !rtvHeap_->Create(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)) {
		return false;
	}

	Log::Send(Log::Level::INFO, "RTV Heap Created");
	
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2];
	for (UINT i = 0; i < 2; ++i){
		if (FAILED(swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources[i])))){
			Log::Send(Log::Level::ERR, "Failed to get swap chain buffer");
			return false;
		}
		swapChainResources_.emplace_back(std::move(swapChainResources[i]));
	}
	Log::Send(Log::Level::INFO, "Swap Chain Resources Created");

	//Set RTVs
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	rtvHandles_[0] = rtvHeap_->Get()->GetCPUDescriptorHandleForHeapStart();
	device_->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc, rtvHandles_[0]);

	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device_->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc, rtvHandles_[1]);

	Log::Send(Log::Level::INFO, "RTVs Created");
	return true;
}

bool DirectXAdapter::CreateFence() {
	if (FAILED(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)))){
		Log::Send(Log::Level::ERR, "Failed to create fence");
		return false;
	}
	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent_ == nullptr){
		Log::Send(Log::Level::ERR, "Failed to create fence event");
		return false;
	}

	fpsLimiter_ = std::make_unique<FrameRateLimiter>(static_cast<uint16_t>(60), true); // 60 FPS, VSync enabled

	Log::Send(Log::Level::INFO, "Fence Created");
	return true;
}

void DirectXAdapter::Wait() {
	++fenceValue_;
	cQueue_->Signal(fence_.Get(), fenceValue_);

	if (fence_->GetCompletedValue() < fenceValue_){
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
		//CloseHandle(fenceEvent_);
	}

	fpsLimiter_->WaitForNextFrame(); 
}

ID3D12Device * DirectXAdapter::GetDevice() const {
	return device_.Get();
}

ID3D12GraphicsCommandList * DirectXAdapter::GetCommandList() const {
	return cList_.Get();
}

size_t DirectXAdapter::GetWidth() const {
	return windowSize_.first;
}

size_t DirectXAdapter::GetHeight() const {
	return windowSize_.second;
}

HWND DirectXAdapter::GetWindowHandle() const {
	return hWnd_;
}
