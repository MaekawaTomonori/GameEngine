#include "Renderer.hpp"

#include <thread>

#include "Log.hpp"
#include "Utils.hpp"

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

Renderer::~Renderer() {
    isRunning_ = false;
    if (thread_.joinable()){
        thread_.join();
    }
    if (fenceEvent_){
        CloseHandle(fenceEvent_);
        fenceEvent_ = nullptr;
    }
    Log::Send(Log::Level::INFO, "Renderer Destroyed");
}

void Renderer::Initialize(const DirectXAdapter *_adapter) {
    isRunning_ = true;
    if (!_adapter){
        Log::Send(Log::Level::ERR, "DirectXAdapter is null");
        Utils::Alert("DirectXAdapter is Null");
        return;
    }

    cQueue_ = _adapter->GetCommandQueue();
    cAllocator_ = _adapter->GetCommandAllocator();
    cList_ = _adapter->GetCommandList();

    swapChain_ = _adapter->GetSwapChain();

    swapChainResources_.clear();
    swapChainResources_.resize(2);

    for (UINT i = 0; i < 2; ++i){
        if (FAILED(swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i])))){
            Log::Send(Log::Level::ERR, "Failed to get swap chain buffer");
            Utils::Alert("Failed to get swap chain buffer");
            return;
        }
    }

    if (!CreateRTV(_adapter->GetDevice())) {
        Log::Send(Log::Level::ERR, "Failed to create RTV");
        Utils::Alert("Failed to create RTV");
        return;
    }

    fence_ = _adapter->GetFence();
    fenceValue_ = 0;
    fenceEvent_ = CreateEvent(nullptr, false, false, nullptr);
    if (fenceEvent_ == nullptr){
        Log::Send(Log::Level::ERR, "Failed to create fence event");
        Utils::Alert("Failed to create fence event");
        return;
    }

    fpsLimiter_ = std::make_unique<FrameRateLimiter>(static_cast<uint16_t>(60), true); // 60 FPS with VSync enabled
    if (!fpsLimiter_){
        Log::Send(Log::Level::ERR, "Failed to create FrameRateLimiter");
        Utils::Alert("Failed to create FrameRateLimiter");
        return;
    }

    RunThread();


    Log::Send(Log::Level::INFO, "Renderer Initialized");
}

void Renderer::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        isRunning_ = false;
    }
	if (thread_.joinable()){
        thread_.join();
    }
    renderingCommands_ = {};
    pendingCommands_ = {};
    Log::Send(Log::Level::INFO, "Renderer Shutdown");
}

void Renderer::Register(std::function<void()> _task) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (_task){
        pendingCommands_.push(std::move(_task));
    } else{
        Log::Send(Log::Level::ERR, "Rendering command is null");
    }
}

void Renderer::Render() {
    condition_.notify_one();
}

void Renderer::Wait() {
    ++fenceValue_;
    cQueue_->Signal(fence_.Get(), fenceValue_);

    if (fence_->GetCompletedValue() < fenceValue_){
        fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
        WaitForSingleObject(fenceEvent_, INFINITE);
        //CloseHandle(fenceEvent_);
    }

    fpsLimiter_->WaitForNextFrame();
}

void Renderer::RunThread() {
    thread_ = std::thread([&](){
        RenderingProcess();
    });
}

void Renderer::RenderingProcess() {
    while (isRunning_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this]{ return !pendingCommands_.empty() || !isRunning_; });

            if (!isRunning_ && renderingCommands_.empty() && pendingCommands_.empty()) break;

            if (renderingCommands_.empty() && !pendingCommands_.empty()) {
	            renderingCommands_ = std::move(pendingCommands_);
            }

        }
        if (!renderingCommands_.empty()) {
            ExecuteCommands();
        }
    }
}

void Renderer::ExecuteCommands() {
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
    while (!renderingCommands_.empty()){
        auto& command = renderingCommands_.front();
    	renderingCommands_.pop();
        if (command){
            command();
        }
    }

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cList_->ResourceBarrier(1, &barrier);

    if (FAILED(cList_->Close())){
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

    if (FAILED(cList_->Reset(cAllocator_.Get(), nullptr))){
        Utils::Alert("Failed to reset command list");
        return;
    }
}

bool Renderer::CreateRTV(ID3D12Device* _device) {
    rtvHeap_ = std::make_unique<Heap>();
    if (!rtvHeap_ || !rtvHeap_->Create(_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)){
        return false;
    }

    Log::Send(Log::Level::INFO, "RTV Heap Created");

    swapChainResources_.resize(2);
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

    rtvHandles_.resize(2);
    rtvHandles_[0] = rtvHeap_->Get()->GetCPUDescriptorHandleForHeapStart();
    _device->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc, rtvHandles_[0]);

    rtvHandles_[1].ptr = rtvHandles_[0].ptr + _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    _device->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc, rtvHandles_[1]);

    Log::Send(Log::Level::INFO, "RTVs Created");
    return true;
}
