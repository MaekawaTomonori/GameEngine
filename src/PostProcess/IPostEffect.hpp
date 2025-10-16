#ifndef IPostEffect_HPP_
#define IPostEffect_HPP_
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"

/// <summary>
/// ポストエフェクト基底インターフェース
/// シェーダーベースのポストプロセス効果を実装するための基底クラス
/// </summary>
class IPostEffect {
protected:
    const Vector4 CLEAR_COLOR = { 0.2f, 0.2f, 0.2f, 1.0f };

    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;

    std::unique_ptr<PipelineStateObject> pso_;
    std::unique_ptr<DX12Resource> output_;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};

    uint32_t index_{};
    D3D12_GPU_DESCRIPTOR_HANDLE handle_{};

public:
    virtual ~IPostEffect() = default;

    /// <summary>
    /// セットアップ
    /// </summary>
    /// <param name="_adapter">DirectXアダプター</param>
    /// <param name="_srv">SRVマネージャー</param>
    void SetUp(DirectXAdapter* _adapter, SRVManager* _srv);

    /// <summary>
    /// 初期化処理（純粋仮想関数）
    /// </summary>
    virtual void Initialize() = 0;

    /// <summary>
    /// ポストエフェクトを適用
    /// </summary>
    /// <param name="_handle">入力GPUハンドル</param>
    /// <returns>出力GPUハンドル</returns>
    D3D12_GPU_DESCRIPTOR_HANDLE Apply(D3D12_GPU_DESCRIPTOR_HANDLE _handle);

    /// <summary>
    /// RTVハンドルを設定
    /// </summary>
    /// <param name="rtvHandle">RTVハンドル</param>
    void SetRTVHandle(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

    /// <summary>
    /// デバッグ情報の表示（純粋仮想関数）
    /// </summary>
    virtual void Debug() = 0;

protected:
    void CreateOutput();
    virtual void Modifier() = 0;
}; // class IPostEffect

#endif // IPostEffect_HPP_
