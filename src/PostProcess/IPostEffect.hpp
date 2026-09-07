#ifndef IPostEffect_HPP_
#define IPostEffect_HPP_
#include <string>
#include <vector>

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

    /** @brief 出力先リソースを取得する（Editorのプレビュー表示等に使う）
     * @return 出力リソース。未初期化の場合は nullptr
     */
    ID3D12Resource* GetOutputResource() const { return output_ ? output_->Get() : nullptr; }

    /** @brief 出力SRVのスロット番号を取得する（削除時にディスクリプタを付け替えるために使う） */
    uint32_t GetSrvIndex() const { return index_; }

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

    /** @brief エフェクトタイプ名を取得
     * Factory登録名・プリセットファイルのパスと一致させる。
     * @return エフェクトタイプ名（例: "Vignette"）
     */
    virtual std::string GetTypeName() const = 0;

    /** @brief 現在稼働中のパラメータ値を1キーフレーム分のJSONとして取得
     * SaveParameters()と異なり、保持中の全キーフレームではなく現在の実行時パラメータのみを返す。
     * @return 現在のパラメータのJSON表現
     */
    virtual nlohmann::json CaptureCurrentParameters() const = 0;

    /** @brief JSONで表現されたパラメータを現在の実行時パラメータへ適用する
     * CaptureCurrentParameters()と対になる。
     * @param _params 適用するパラメータのJSON表現
     */
    virtual void ApplyParameters(const nlohmann::json& _params) = 0;

protected:
    /** @brief 出力リソースの作成
     */
    void CreateOutput();

    virtual void Modifier() = 0;

    /** @brief プリセットファイルのパスを組み立てる
     * @param _presetName プリセット名
     * @return "./Assets/Data/PostEffect/<GetTypeName()>/<_presetName>.json"
     */
    std::string BuildPresetPath(const std::string& _presetName) const;

    /** @brief キーフレームファイルを読み込む
     * @param _presetName プリセット名
     * @param _outRaw "keyframes"を除いた生JSON（キーフレーム名 -> パラメータJSON）
     * @param _outOrder キーフレームの並び順
     * @return ファイルが存在し読み込めた場合はtrue。存在しない場合はfalse（呼び出し側でデフォルト値を適用する）
     */
    bool LoadKeyframeFile(const std::string& _presetName, nlohmann::json& _outRaw, std::vector<std::string>& _outOrder) const;

    /** @brief キーフレームファイルを保存する
     * @param _presetName プリセット名
     * @param _keyframesObject キーフレーム名 -> パラメータJSON
     * @param _order キーフレームの並び順
     */
    void SaveKeyframeFile(const std::string& _presetName, const nlohmann::json& _keyframesObject, const std::vector<std::string>& _order) const;
}; // class IPostEffect

#endif // IPostEffect_HPP_
