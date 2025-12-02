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

/** @brief DirectX12アダプタークラス
 ** DirectX12の初期化とデバイス管理を提供
 **/
class DirectXAdapter {
    /** @brief first = width, second = height
     **/
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
    Vector4 back_ = {0.2f, 0.2f, 0.2f, 1.0f}; // Black

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

    /** @brief バッファリソースを作成
     ** @param _size バッファサイズ
     ** @return 作成されたリソース
     **/
    std::unique_ptr<DX12Resource> CreateBufferResource(size_t _size) const;

    /** @brief テクスチャリソースを作成
     ** @param metadata テクスチャメタデータ
     ** @return 作成されたリソース
     **/
    std::unique_ptr<DX12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata) const;

    /** @brief 深度ステンシルリソースを作成
     ** @param _width 幅
     ** @param _height 高さ
     ** @return 作成されたリソース
     **/
    std::unique_ptr<DX12Resource> CreateDepthStencilResource(int32_t _width, int32_t _height) const;

    /** @brief レンダーテクスチャリソースを作成
     ** @param _width 幅
     ** @param _height 高さ
     ** @param _format フォーマット
     ** @param _cc クリアカラー
     ** @return 作成されたリソース
     **/
    std::unique_ptr<DX12Resource> CreateRenderTextureResource(uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _cc) const;

    std::unique_ptr<DX12Resource> CreateUnorderedAccessView() const;

    /** @brief プリプロセス処理
     **/
    void PreProcess() const;

    /** @brief フレームの開始処理
     **/
    void BeginFrame();

    /** @brief フレームの終了処理
     **/
    void EndFrame();

    /** @brief FPSを表示
     ** @param _debug デバッグUI
     **/
    void DisplayFPS(DebugUI* _debug) const;

    /** @brief ウィンドウサイズを更新（ビューポートとシザー矩形も更新）
     ** @param _width 新しい幅
     ** @param _height 新しい高さ
     **/
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

    /** @brief GPU同期を実行（RAII化のためのヘルパー）
     **/
    void SyncGPU();

    /** @brief SwapChainリソースを安全に解放（RAII化のためのヘルパー）
     **/
    void ReleaseSwapChainResources();

    /** @brief RTV記述子を作成（Single Responsibility）
     **/
    void CreateRTVDescriptors();

    /** @brief DSV記述子を作成（Single Responsibility）
     **/
    void CreateDSVDescriptor();

public: //Accessor
    /** @brief ウィンドウハンドルを取得
     ** @return ウィンドウハンドル
     **/
    [[nodiscard]] HWND GetWindowHandle() const;

    /** @brief DirectX12デバイスを取得
     ** @return デバイスポインタ
     **/
    [[nodiscard]] ID3D12Device* GetDevice() const;

    /** @brief コマンドリストを取得
     ** @return コマンドリストポインタ
     **/
    [[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const;

    /** @brief ウィンドウ幅を取得
     ** @return 幅
     **/
    size_t GetWidth() const;

    /** @brief ウィンドウ高さを取得
     ** @return 高さ
     **/
    size_t GetHeight() const;

    /** @brief コマンドキューを取得
     ** @return コマンドキューポインタ
     **/
    ID3D12CommandQueue* GetCommandQueue() const;

    /** @brief コマンドアロケーターを取得
     ** @return コマンドアロケーターポインタ
     **/
    ID3D12CommandAllocator* GetCommandAllocator() const;

    /** @brief スワップチェーンを取得
     ** @return スワップチェーンポインタ
     **/
    IDXGISwapChain4* GetSwapChain() const;

    /** @brief フェンスを取得
     ** @return フェンスポインタ
     **/
    ID3D12Fence* GetFence() const;

    /** @brief DSVハンドルを取得
     ** @return DSVハンドル
     **/
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const;

    /** @brief 次のフェンス値を取得
     ** @return フェンス値
     **/
    uint64_t GetNextFenceValue();

    /** @brief フェンス値まで待機
     ** @param _fenceValue 待機するフェンス値
     **/
    void WaitForFenceValue(uint64_t _fenceValue) const;
}; // class DirectXAdapter

#endif // DirectXAdaptor_HPP_
