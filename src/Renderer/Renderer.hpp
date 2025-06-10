#ifndef Renderer_HPP_
#define Renderer_HPP_

#include <chrono>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
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

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable condition_;
	bool isRunning_ = false;
    

    //Command
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> cQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cList_;

    //SwapChain
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

    //Resource
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> swapChainResources_;

    //RTV
    std::unique_ptr<Heap> rtvHeap_;

    //RtvHandle
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles_;

    //Background color
    Vector4	back = {0.2f, 0.2f, 0.2f, 1.0f}; // Black

    //Fence
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;

    //RenderingCommand
    std::queue<std::function<void()>> renderingCommands_;
    std::queue<std::function<void()>> pendingCommands_;

    std::unique_ptr<FrameRateLimiter> fpsLimiter_ = nullptr;

public:
	~Renderer();
	void Initialize(const DirectXAdapter* _adapter);
    void Shutdown();

    void Register(std::function<void()> _task);
    void Render();
private:
	bool CreateRTV(ID3D12Device* _device);
	void Wait();

    // thread
    void RunThread();
    // worker loop
    void RenderingProcess();
    // frame rendering
    void ExecuteCommands();

}; // class Renderer

#endif // Renderer_HPP_
