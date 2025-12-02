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

    /// <summary>
    /// バッファリソースを作成
    /// </summary>
    /// <param name="_size">バッファサイズ</param>
    /// <returns>作成されたリソース</returns>
    std::unique_ptr<DX12Resource> CreateBufferResource(size_t _size) const;

    /// <summary>
    /// テクスチャリソースを作成
    /// </summary>
    /// <param name="metadata">テクスチャメタデータ</param>
    /// <returns>作成されたリソース</returns>
    std::unique_ptr<DX12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata) const;

    /// <summary>
    /// 深度ステンシルリソースを作成
    /// </summary>
    /// <param name="_width">幅</param>
    /// <param name="_height">高さ</param>
    /// <returns>作成されたリソース</returns>
    std::unique_ptr<DX12Resource> CreateDepthStencilResource(int32_t _width, int32_t _height) const;

    /// <summary>
    /// レンダーテクスチャリソースを作成
    /// </summary>
    /// <param name="_width">幅</param>
    /// <param name="_height">高さ</param>
    /// <param name="_format">フォーマット</param>
    /// <param name="_cc">クリアカラー</param>
    /// <returns>作成されたリソース</returns>
    std::unique_ptr<DX12Resource> CreateRenderTextureResource(uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _cc) const;

    std::unique_ptr<DX12Resource> CreateUnorderedAccessView() const;

    /// <summary>
    /// プリプロセス処理
    /// </summary>
    void PreProcess() const;

    /// <summary>
    /// フレームの開始処理
    /// </summary>
    void BeginFrame();

    /// <summary>
    /// フレームの終了処理
    /// </summary>
    void EndFrame();

    /// <summary>
    /// FPSを表示
    /// </summary>
    /// <param name="_debug">デバッグUI</param>
    void DisplayFPS(DebugUI* _debug) const;

    /// <summary>
    /// ウィンドウサイズを更新（ビューポートとシザー矩形も更新）
    /// </summary>
    /// <param name="_width">新しい幅</param>
    /// <param name="_height">新しい高さ</param>
    void UpdateWindowSize(size_t _width, size_t _height);

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
    /// <summary>
    /// ウィンドウハンドルを取得
    /// </summary>
    /// <returns>ウィンドウハンドル</returns>
    [[nodiscard]] HWND GetWindowHandle() const;

    /// <summary>
    /// DirectX12デバイスを取得
    /// </summary>
    /// <returns>デバイスポインタ</returns>
    [[nodiscard]] ID3D12Device* GetDevice() const;

    /// <summary>
    /// コマンドリストを取得
    /// </summary>
    /// <returns>コマンドリストポインタ</returns>
    [[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const;

    /// <summary>
    /// ウィンドウ幅を取得
    /// </summary>
    /// <returns>幅</returns>
    size_t GetWidth() const;

    /// <summary>
    /// ウィンドウ高さを取得
    /// </summary>
    /// <returns>高さ</returns>
    size_t GetHeight() const;

    /// <summary>
    /// コマンドキューを取得
    /// </summary>
    /// <returns>コマンドキューポインタ</returns>
    ID3D12CommandQueue* GetCommandQueue() const;

    /// <summary>
    /// コマンドアロケーターを取得
    /// </summary>
    /// <returns>コマンドアロケーターポインタ</returns>
    ID3D12CommandAllocator* GetCommandAllocator() const;

    /// <summary>
    /// スワップチェーンを取得
    /// </summary>
    /// <returns>スワップチェーンポインタ</returns>
    IDXGISwapChain4* GetSwapChain() const;

    /// <summary>
    /// フェンスを取得
    /// </summary>
    /// <returns>フェンスポインタ</returns>
    ID3D12Fence* GetFence() const;

    /// <summary>
    /// DSVハンドルを取得
    /// </summary>
    /// <returns>DSVハンドル</returns>
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const;

    /// <summary>
    /// 次のフェンス値を取得
    /// </summary>
    /// <returns>フェンス値</returns>
    uint64_t GetNextFenceValue();

    /// <summary>
    /// フェンス値まで待機
    /// </summary>
    /// <param name="_fenceValue">待機するフェンス値</param>
    void WaitForFenceValue(uint64_t _fenceValue) const;
}; // class DirectXAdapter

#endif // DirectXAdaptor_HPP_
