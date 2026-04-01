#ifndef IPostEffect_HPP_
#define IPostEffect_HPP_
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "json.hpp"

/** @brief ポストエフェクト基底インターフェース
 * シェーダーベースのポストプロセス効果を実装するための基底クラス
 */
class IPostEffect {
protected:
    const Vector4 CLEAR_COLOR = { 0.2f, 0.2f, 0.2f, 1.0f };

    GESTD::ReferencePtr<DirectXAdapter> adapter_ = nullptr;
    GESTD::ReferencePtr<SRVManager> srv_ = nullptr;

    std::unique_ptr<PipelineStateObject> pso_;
    std::unique_ptr<DX12Resource> output_;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};

    uint32_t index_{};
    D3D12_GPU_DESCRIPTOR_HANDLE handle_{};

public:
    virtual ~IPostEffect() = default;

    /** @brief セットアップ
     * @param _adapter DirectXアダプター
     * @param _srv SRVマネージャー
     */
    void SetUp(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv);

    /** @brief 初期化処理（純粋仮想関数）
     */
    virtual void Initialize() = 0;

    /** @brief ポストエフェクトを適用
     * @param _handle 入力GPUハンドル
     * @return 出力GPUハンドル
     */
    D3D12_GPU_DESCRIPTOR_HANDLE Apply(D3D12_GPU_DESCRIPTOR_HANDLE _handle);

    /** @brief RTVハンドルを設定
     * @param _rtvHandle RTVハンドル
     */
    void SetRTVHandle(D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle);

    /** @brief デバッグ情報の表示（純粋仮想関数）
     */
    virtual void Debug() = 0;

    /** @brief プリセットからパラメータを読み込み
     * @param _presetName プリセット名
     */
    virtual void LoadPreset(const std::string& _presetName) = 0;

    /** @brief 現在のパラメータをプリセットとして保存
     * @param _presetName プリセット名
     */
    virtual void SavePreset(const std::string& _presetName) = 0;

    /** @brief 現在のパラメータをJSONに変換
     * @return パラメータのJSON表現
     */
    virtual nlohmann::json SaveParameters() const = 0;

    /** @brief アニメーション進行（tは0.0~1.0）
     * @param _t アニメーション進行度（0.0: 開始、1.0: 終了）
     */
    virtual void UpdateAnimation(float _t) = 0;

protected:
    /** @brief 出力リソースの作成
     */
    void CreateOutput();

    virtual void Modifier() = 0;
}; // class IPostEffect

#endif // IPostEffect_HPP_
