#include "DirectXAdaptor.hpp"

#include "include/Log.hpp"
#include "include/Utils.hpp"
#include "src/Platform/WinApp.hpp"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

DirectXAdaptor::DirectXAdaptor(HWND _hWnd, size_t _width, size_t _height) :hWnd_(_hWnd), windowSize_(_width, _height) {
	if (!CreateDXGI())Utils::Alert("Failed to create DXGI");
	if (!CreateCommand())Utils::Alert("Failed to create Command");

}

bool DirectXAdaptor::CreateDXGI() {
	Log::Send(Log::Level::INFO, "Create DXGI");

	//Factory
	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory_)))){
		return false;
	}

	Log::Send(Log::Level::INFO, "Factory Created");

	//Adaptor
	for (UINT i = 0; factory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter_)); ++i){
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

bool DirectXAdaptor::CreateCommand() {
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

bool DirectXAdaptor::CreateSwapChain() const {
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.Width = static_cast<UINT>(windowSize_.first);
	desc.Height = static_cast<UINT>(windowSize_.second);
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	if (FAILED(factory_->CreateSwapChainForHwnd(cQueue_.Get(), hWnd_, &desc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1 **>(swapChain_.Get())))) {
		Log::Send(Log::Level::ERR, "Failed to create swap chain");
		return false;
	}
	Log::Send(Log::Level::INFO, "Swap Chain Created");

	return true;
}
