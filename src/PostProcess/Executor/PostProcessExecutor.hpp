#ifndef PostProcessExecutor_HPP_
#define PostProcessExecutor_HPP_
#include <vector>
#include <memory>
#include <functional>
#include <d3d12.h>

#include "ReferencePtr.hpp"
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

    GESTD::ReferencePtr<DirectXAdapter> adapter_;
    GESTD::ReferencePtr<SRVManager> srv_ = nullptr;
    GESTD::ReferencePtr<DebugUI> debugUI_;
    GESTD::ReferencePtr<AbstractPostEffectFactory> factory_ = nullptr;  // PostEffect factory

    std::vector<EffectData> effects_;

    /** シーン描画用 RenderTexture */
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

    /** Animation state */
    bool isAnimating_ = false;
    float animationTimer_ = 0.f;
    float animationDuration_ = 0.f;
    std::vector<std::string> animatingEffects_;  // animation target effect names
    std::function<void()> onAnimationComplete_;  // animation completion callback

    /** Preset editor */
    std::unique_ptr<PostProcessPresetEditor> presetEditor_;

    /** ImGui scene view 用 GPU texture ID */
    uint64_t sceneImGuiTextureId_ = 0;

    /** SceneView 表示スケール */
    float sceneViewScale_ = 1.0f;

public:
    /** @brief 初期化
     * <param name=_adapter">DirectX adapter</param>
     * <param name=_srv">SRV manager</param>
     * <param name=_debug">Debug UI</param>
     */
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv, const GESTD::ReferencePtr<DebugUI>& _debug);

    /** @brief PostEffect factory を設定 */
    void SetFactory(GESTD::ReferencePtr<AbstractPostEffectFactory> _factory);

    /** @brief エフェクトを追加
     * @param _effect 追加するエフェクト
     * @param _name エフェクト名
     */
    void Add(std::unique_ptr<IPostEffect> _effect, const std::string& _name = "NoName");

    /** @brief フレーム開始時の描画準備 */
    void BeginFrame() const;

    /** @brief フレーム終了時のクリーンアップ */
    void EndFrame() const;

    /** @brief エフェクトを実行 */
    void Execute();

    /** @brief エフェクト結果を描画 */
    void Draw() const;

    /** @brief エフェクトの有効/無効を切り替える
     * @param _name エフェクト名
     * @param _enable 有効フラグ
     */
    void SetActive(const std::string& _name, bool _enable);

    /** @brief デバッグ UI を表示 */
    void Debug();

    /** @brief preset を適用する
     * @param _presetName preset 名
     * @param _mode "add" または "replace"
     * @param _ignoreList 無視する effect 名
     * @param _onComplete 完了時コールバック
     */
    void ApplyPreset(
        const std::string& _presetName,
        const std::string& _mode = "replace",
        const std::vector<std::string>& _ignoreList = {},
        std::function<void()> _onComplete = nullptr
    );

    /** @brief アニメーションを更新
     * @param _deltaTime 経過時間
     */
    void Update(float _deltaTime);

    /** @brief 現在の active effect 構成を JSON に保存
     * @param _presetName 保存する preset 名
     */
    void SavePreset(const std::string& _presetName);

    /** @brief preset editor を開く
     * @param _presetName 編集対象 preset 名
     */
    void OpenPresetEditor(const std::string& _presetName = "");

    /** @brief エフェクトを検索し、必要なら生成する
     * @param _type effect type
     * @param _name instance 名
     * @param _create 見つからないときに生成するか
     * @return effect instance。失敗時は nullptr
     */
    IPostEffect* FindOrCreate(const std::string& _type, const std::string& _name, bool _create);

    /** @brief render texture をリサイズする */
    void ResizeRenderTextures();

    /** @brief SceneView が表示中かどうか */
    bool IsSceneViewActive();

    /** @brief SceneView の表示スケールを取得 */
    float GetSceneViewScale() const { return sceneViewScale_; }

private:
    /** @brief シーン描画用 render texture を作成 */
    void CreateSceneRenderTexture();

    /** @brief RTV/SRV を作成 */
    void CreateRenderTextureViews();
}; // class PostProcessExecutor

#endif // PostProcessExecutor_HPP_
