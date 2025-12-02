#ifndef IPostEffect_HPP_
#define IPostEffect_HPP_
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "vendor/json/json.hpp"

/** @brief ポストエフェクト基底インターフェース
 ** シェーダーベースのポストプロセス効果を実装するための基底クラス
 **/
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

    /** @brief セットアップ
     ** @param _adapter DirectXアダプター
     ** @param _srv SRVマネージャー
     **/
    void SetUp(DirectXAdapter* _adapter, SRVManager* _srv);

    /** @brief 初期化処理（純粋仮想関数）
     **/
    virtual void Initialize() = 0;

    /** @brief ポストエフェクトを適用
     ** @param _handle 入力GPUハンドル
     ** @return 出力GPUハンドル
     **/
    D3D12_GPU_DESCRIPTOR_HANDLE Apply(D3D12_GPU_DESCRIPTOR_HANDLE _handle);

    /** @brief RTVハンドルを設定
     ** @param rtvHandle RTVハンドル
     **/
    void SetRTVHandle(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

    /** @brief デバッグ情報の表示（純粋仮想関数）
     **/
    virtual void Debug() = 0;

    /** @brief プリセットからパラメータを読み込み
     ** @param presetName プリセット名
     **/
    virtual void LoadPreset(const std::string& presetName) = 0;

    /** @brief 現在のパラメータをプリセットとして保存
     ** @param presetName プリセット名
     **/
    virtual void SavePreset(const std::string& presetName) = 0;

    /** @brief 現在のパラメータをJSONに変換
     ** @return パラメータのJSON表現
     **/
    virtual nlohmann::json SaveParameters() const = 0;

    /** @brief アニメーション進行（tは0.0~1.0）
     ** @param t アニメーション進行度（0.0: 開始、1.0: 終了）
     **/
    virtual void UpdateAnimation(float t) = 0;

    ///// <summary>
    ///// キーフレーム名リストを取得
    ///// </summary>
    ///// <returns>キーフレーム名の配列</returns>
    //virtual std::vector<std::string> GetKeyframeNames() const = 0;
    //
    ///// <summary>
    ///// キーフレームの順序を取得
    ///// </summary>
    ///// <returns>キーフレーム名の順序配列</returns>
    //virtual std::vector<std::string> GetKeyframeOrder() const = 0;
    //
    ///// <summary>
    ///// キーフレームの順序を設定
    ///// </summary>
    ///// <param name="order">新しい順序配列</param>
    //virtual void SetKeyframeOrder(const std::vector<std::string>& order) = 0;
    //
    ///// <summary>
    ///// 新しいキーフレームを追加
    ///// </summary>
    ///// <param name="name">キーフレーム名</param>
    ///// <returns>追加成功ならtrue</returns>
    //virtual bool AddKeyframe(const std::string& name) = 0;
    //
    ///// <summary>
    ///// キーフレームを削除
    ///// </summary>
    ///// <param name="name">キーフレーム名</param>
    ///// <returns>削除成功ならtrue</returns>
    //virtual bool RemoveKeyframe(const std::string& name) = 0;
    //
    ///// <summary>
    ///// キーフレーム名を変更
    ///// </summary>
    ///// <param name="oldName">元の名前</param>
    ///// <param name="newName">新しい名前</param>
    ///// <returns>変更成功ならtrue</returns>
    //virtual bool RenameKeyframe(const std::string& oldName, const std::string& newName) = 0;
    //
    ///// <summary>
    ///// 指定したキーフレームのパラメータをImGuiで編集
    ///// </summary>
    ///// <param name="keyframeName">編集対象のキーフレーム名</param>
    //virtual void EditKeyframe(const std::string& keyframeName) = 0;
    //
    ///// <summary>
    ///// 指定したキーフレームのパラメータを現在の状態に適用（プレビュー用）
    ///// </summary>
    ///// <param name="keyframeName">適用するキーフレーム名</param>
    //virtual void ApplyKeyframe(const std::string& keyframeName) = 0;
    //
    ///// <summary>
    ///// 現在のエフェクト状態をキーフレームとしてキャプチャ
    ///// </summary>
    ///// <param name="keyframeName">保存先キーフレーム名</param>
    //virtual void CaptureCurrentStateAsKeyframe(const std::string& keyframeName) = 0;

protected:
    void CreateOutput();
    virtual void Modifier() = 0;
}; // class IPostEffect

#endif // IPostEffect_HPP_
