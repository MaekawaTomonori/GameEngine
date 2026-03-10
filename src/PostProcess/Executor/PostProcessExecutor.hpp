#ifndef PostProcessExecutor_HPP_
#define PostProcessExecutor_HPP_
#include <vector>
#include <memory>
#include <functional>
#include <d3d12.h>

#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/PostProcess/IPostEffect.hpp"
#include "src/PostProcess/Editor/PostProcessPresetEditor.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

class DirectXAdapter;
class Heap;
class AbstractPostEffectFactory;

class PostProcessExecutor {
    struct EffectData {
        std::unique_ptr<IPostEffect> effect;
        std::string name;
        std::string type;
        bool enabled = true;
    };

    struct Work {
        struct Member {
            std::string name;
            std::string type;
            bool autoCreate;
        };

        std::string name;
        float duration;
        std::vector<Member> members;
        std::vector<std::string> ignore;
    };

    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;
    DebugUI* debugUI_ = nullptr;
    AbstractPostEffectFactory* factory_ = nullptr;  // PostEffectファクトリー

    std::vector<EffectData> effects_;

    /** シーン描画用RenderTexture
     */
    std::unique_ptr<DX12Resource> renderTexture_;
    std::unique_ptr<Heap> rtvHeap_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle_{};

    std::unique_ptr<PipelineStateObject> pso_;

    Vector4 clearColor_{ 0.2f, 0.2f, 0.2f, 1.0f };
    uint32_t srvIndex_ = 0;

    std::unique_ptr<DX12Resource> tempBuffer_;

    std::vector<Work> works_;
    float currentTimer_ = 0.f;
    float duration_ = 0.f;
    std::string currentWork_ {};

    /** Animation state
     */
    bool isAnimating_ = false;
    float animationTimer_ = 0.f;
    float animationDuration_ = 0.f;
    std::vector<std::string> animatingEffects_;  // エフェクト名リスト
    std::function<void()> onAnimationComplete_;  // アニメーション完了コールバック

    /** Preset editor
     */
    std::unique_ptr<PostProcessPresetEditor> presetEditor_;

    /** ImGui シーンビュー用 GPU テクスチャ ID（DebugUI ヒープのスロット1に対応）
     */
    uint64_t sceneImGuiTextureId_ = 0;

    /** シーンビューの表示スケール（displaySize / renderSize）
     */
    float sceneViewScale_ = 1.0f;

public:
    /** @brief 初期化処理
     * <param name=_adapter"> DirectXアダプター</param>
     *  <param name=_srv"> SRVマネージャー</param>
     * <param name=_debug"> デバッグUI</param>
     */
    void Initialize(DirectXAdapter* _adapter, SRVManager* _srv, DebugUI* _debug);

    /** @brief ポストエフェクトファクトリーの設定
     */
    void SetFactory(AbstractPostEffectFactory* _factory);

    /** @brief  エフェクトの追加
     * @param _effect 追加するエフェクトのユニークポインタ
     * @param _name エフェクト名
     */
    void Add(std::unique_ptr<IPostEffect> _effect, const std::string& _name = "NoName");

    /** @brief フレーム開始前の準備処理
     */
    void BeginFrame() const;

    /** @brief フレーム終了後のクリーンアップ処理
     */
    void EndFrame() const;

    /** @brief エフェクトの実行
     */
    void Execute();

    /** @brief エフェクトの描画
     */
    void Draw() const;

    /** @brief エフェクトの有効/無効を切り替え
     * @param _name エフェクト名
     * @param _enable 有効/無効フラグ
     */
    void SetActive(const std::string& _name, bool _enable);

    /** @brief エフェクトのデバッグ情報を表示
     */
    void Debug();

    /** @brief プリセットを適用（遅延初期化 + アニメーション開始）
     * @param _presetName プリセット名（presets.jsonのキー）
     * @param _mode "add"または"replace"
     * @param _ignoreList 無視するエフェクト名リスト
     * @param _onComplete アニメーション完了時のコールバック（オプション）
     */
    void ApplyPreset(
        const std::string& _presetName,
        const std::string& _mode = "replace",
        const std::vector<std::string>& _ignoreList = {},
        std::function<void()> _onComplete = nullptr
    );

    /** @brief アニメーション更新（毎フレーム呼び出し）
     * @param _deltaTime フレーム間時間
     */
    void Update(float _deltaTime);

    /** @brief 現在のアクティブエフェクトの状態をJSONに保存
     * @param _presetName 保存先プリセット名
     */
    void SavePreset(const std::string& _presetName);

    /** @brief プリセットエディターを開く
     * @param _presetName 編集するプリセット名（空なら新規作成）
     */
    void OpenPresetEditor(const std::string& _presetName = "");

    /** @brief エフェクトを検索、なければ生成
    * @param _type エフェクトタイプ名
    * @param _name エフェクトインスタンス名
    * @param _create インスタンスが存在しない場合に生成するか
    * @return エフェクトポインタ、生成失敗時はnullptr
    */
    IPostEffect* FindOrCreate(const std::string& _type, const std::string& _name, bool _create);

    /** @brief レンダーテクスチャをリサイズ（ウィンドウリサイズ時に呼び出し）
     */
    void ResizeRenderTextures();

    /** @brief Scene ウィンドウが表示中かを返す（Renderer のキャプチャモード判定用）
     */
    bool IsSceneViewActive();

    /** @brief 現在のシーンビュー表示スケールを返す（表示サイズ / レンダーサイズ）
     */
    float GetSceneViewScale() const { return sceneViewScale_; }

private:
    /** @brief シーン描画用のレンダーテクスチャを作成
     */
    void CreateSceneRenderTexture();

    /** @brief RTV/SRV記述子を作成（DRY原則に基づく共通化）
     */
    void CreateRenderTextureViews();
}; // class PostProcessExecutor

#endif // PostProcessExecutor_HPP_
