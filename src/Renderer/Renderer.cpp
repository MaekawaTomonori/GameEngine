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

void Renderer::Initialize(DirectXAdapter *_adapter) {
}

void Renderer::Register(std::function<void()> _task) {
    if (_task){
        renderingCommands_.emplace_back(std::move(_task));
    } else{
        Log::Send(Log::Level::ERR, "Rendering command is null");
    }
}

void Renderer::Render() {
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
