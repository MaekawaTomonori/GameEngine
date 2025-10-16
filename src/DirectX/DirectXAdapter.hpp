#ifndef DirectXAdapter_HPP_
#define DirectXAdapter_HPP_

#include <d3d12.h>
#include <dxgi1_6.h>
#include <functional>
#include <memory>
#include <queue>
#include <utility>
#include <vector>
#include <wrl/client.h>

#include "FrameRate/FrameRateLimiter.hpp"
#include "Heap/Heap.hpp"
#include "Math/Vector4.hpp"
#include "Resource/DX12Resource.hpp"
#include "vendor/DirectXTex/DirectXTex.h"

class DebugUI;

/// <summary>
/// DirectX12アダプタークラス
/// DirectX12の初期化とデバイス管理を提供
/// </summary>
class DirectXAdapter {
    /// <summary>
    /// first = width, second = height
    /// </summary>
    using WindowSize = std::pair<size_t, size_t>;
    WindowSize windowSize_ = {1280, 720};
    HWND hWnd_ = nullptr;

    //DegubLayer
    Microsoft::WRL::ComPtr<ID3D12Debug1> debugLayer_;

    //DXGIs
    Microsoft::WRL::ComPtr<IDXGIFactory7> factory_;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter_;
    Microsoft::WRL::ComPtr<ID3D12Device> device_;

    //Command
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> cQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cList_;

    //SwapChain
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

    //Resource
    std::vector<std::unique_ptr<DX12Resource>> swapChainResources_;

    //Fence
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;

    //RTV
    std::unique_ptr<Heap> rtvHeap_;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles_;

    //DSV
    std::unique_ptr<DX12Resource> depthStencil_;
    std::unique_ptr<Heap> dsvHeap_;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;

    //Background color
    Vector4 back = {0.2f, 0.2f, 0.2f, 1.0f}; // Black

    //Viewport
    D3D12_VIEWPORT viewport_ = {};

    //Scissor
    D3D12_RECT scissorRect_ = {};

    //Limiter
    std::unique_ptr<FrameRateLimiter> fpsLimiter_ = nullptr;

    //Tasks
    std::queue<std::function<void()>> tasks_;
    std::queue<std::function<void()>> pending_;

    //Executing Flag
    bool isRunning_ = false;

public:
    DirectXAdapter(HWND _hWnd, size_t _width, size_t _height);
    ~DirectXAdapter();

    std::unique_ptr<DX12Resource> CreateBufferResource(size_t _size) const;
    std::unique_ptr<DX12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata) const;
    std::unique_ptr<DX12Resource> CreateDepthStencilResource(int32_t _width, int32_t _height) const;
    std::unique_ptr<DX12Resource> CreateRenderTextureResource(uint32_t _width, uint32_t _height, DXGI_FORMAT _format,
        const Vector4& _cc) const;

    void PreProcess() const;

    // Renderer用のメソッド
    void BeginFrame();
    void EndFrame();

    void DisplayFPS(DebugUI* _debug) const;

private:
    void EnableDebugLayer();
    bool CreateDXGI();
    [[nodiscard]] bool InfoQueue() const;
    bool CreateCommand();
    bool CreateSwapChain();
    bool CreateFence();
    bool CreateRTV();
    bool CreateDSV();
    bool CreateViewportAndScissor();
    bool CreateLimiter();

    void SetSwapChainRenderTarget() const;
    void Present();
    void Wait();

public: //Accessor
    [[nodiscard]] HWND GetWindowHandle() const;
    [[nodiscard]] ID3D12Device* GetDevice() const;
    [[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const;
    size_t GetWidth() const;
    size_t GetHeight() const;
    ID3D12CommandQueue* GetCommandQueue() const;
    ID3D12CommandAllocator* GetCommandAllocator() const;
    IDXGISwapChain4* GetSwapChain() const;
    ID3D12Fence* GetFence() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const;

    // Fence値管理のためのメソッド
    uint64_t GetNextFenceValue();
    void WaitForFenceValue(uint64_t _fenceValue) const;
}; // class DirectXAdapter

#endif // DirectXAdaptor_HPP_
