#ifndef DirectXAdapter_HPP_
#define DirectXAdapter_HPP_

#include <d3d12.h>
#include <dxgi1_6.h>
#include <utility>
#include <vector>
#include <wrl/client.h>


class DirectXAdapter{
    using WindowSize = std::pair<size_t, size_t>;
    WindowSize windowSize_ = {800, 600};
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
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> swapChainResources_;

    //Fence
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    HANDLE fenceEvent_ = nullptr;

public:
    DirectXAdapter(HWND _hWnd, size_t _width, size_t _height);

    ID3D12Resource* CreateBufferResource(size_t _size) const;
private:
    void EnableDebugLayer();
    bool CreateDXGI();
    bool InfoQueue() const;
    bool CreateCommand();
    bool CreateSwapChain();
    bool CreateFence();
public: //Accessor
    [[nodiscard]] HWND GetWindowHandle() const;
    [[nodiscard]] ID3D12Device *GetDevice() const;
    [[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const;
    size_t GetWidth() const;
    size_t GetHeight() const;
    ID3D12CommandQueue* GetCommandQueue() const;
    ID3D12CommandAllocator* GetCommandAllocator() const;
    IDXGISwapChain4* GetSwapChain() const;
    ID3D12Fence* GetFence() const;
}; // class DirectXAdapter

#endif // DirectXAdaptor_HPP_
