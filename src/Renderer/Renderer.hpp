#ifndef Renderer_HPP_
#define Renderer_HPP_

#include <chrono>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <functional>
#include <wrl/client.h>

#include "DirectX/DirectXAdapter.hpp"
#include "DirectX/Heap/Heap.hpp"
#include "Math/Vector4.hpp"

class FrameRateLimiter{
    uint16_t maxFps_;
    std::chrono::steady_clock::time_point reference_;
    bool vsyncEnabled_;

public:
    explicit FrameRateLimiter(uint16_t maxFps, bool useVsync = true);

    void WaitForNextFrame();
};

class Renderer {

    //Command
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> cQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cList_;

    //SwapChain
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

    //RTV
    std::unique_ptr<Heap> rtvHeap_;

    //Resource
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> swapChainResources_;

    //RtvHandle
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

    //Background color
    Vector4	back = {0.2f, 0.2f, 0.2f, 1.0f}; // Black

    //Fence
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;

    //RenderingCommand
    std::vector<std::function<void()>> renderingCommands_;

    std::unique_ptr<FrameRateLimiter> fpsLimiter_ = nullptr;

public:
	void Initialize(DirectXAdapter* _adapter);

    void Register(std::function<void()> _task);
private:
    void Render();
	void Wait();
}; // class Renderer

#endif // Renderer_HPP_
