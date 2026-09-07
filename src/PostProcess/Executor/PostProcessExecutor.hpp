#ifndef PostProcessExecutor_HPP_
#define PostProcessExecutor_HPP_
#include <vector>
#include <memory>
#include <functional>
#include <utility>
#include <d3d12.h>

#include "ReferencePtr.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/PostProcess/IPostEffect.hpp"
#include "src/PostProcess/Chain/PostEffectChain.hpp"
#include "src/PostProcess/Editor/PostProcessPresetEditor.hpp"
#include "src/Canvas/CanvasLayer.hpp"
#include "src/Canvas/Editor/CanvasLayerEditor.hpp"

class DirectXAdapter;
class PostEffectFactory;
class Canvas;

class PostProcessExecutor {
    /** @brief ワンショット演出用の一時エフェクト。再生完了時に破棄される */
    struct TemporaryEffectData {
        std::unique_ptr<IPostEffect> effect;
        std::string type;
        uint32_t rtvSlot = 0;
        uint32_t id = 0;
        bool enabled = true;
    };

    GESTD::ReferencePtr<DirectXAdapter> adapter_;
    GESTD::ReferencePtr<SRVManager> srv_ = nullptr;
    GESTD::ReferencePtr<DebugUI> debugUI_;
    GESTD::ReferencePtr<PostEffectFactory> factory_ = nullptr;  // PostEffect factory

    /** 合成後の画面に適用する常時構成チェーン */
    std::unique_ptr<PostEffectChain> chain_;

    /** 複数Canvasの前後関係を管理するLayer */
    CanvasLayer layer_;

    /** どのエフェクト（常時・ワンショット問わず）も一切受けないCanvas。CanvasLayerには含まれず、最終描画時に直接スワップチェーンへ合成される */
    std::unique_ptr<Canvas> noneCanvas_;

    /** DebugUIのSceneプレビュー用。最終合成結果（chain_ + noneCanvas_）をスワップチェーンとは別にもう一度描いておく */
    std::unique_ptr<PostEffectChain> sceneCaptureChain_;

    std::vector<TemporaryEffectData> temporaries_;
    uint32_t nextTemporaryId_ = 1;

    /** Animation state */
    bool isAnimating_ = false;
    float animationTimer_ = 0.f;
    float animationDuration_ = 0.f;
    std::vector<uint32_t> animatingTemporaries_;  // animation target temporary effect ids
    std::function<void()> onAnimationComplete_;  // animation completion callback

    /** Preset editor（ワンショット演出用） */
    std::unique_ptr<PostProcessPresetEditor> presetEditor_;

    /** Canvas Editor（Canvas常時構成用） */
    std::unique_ptr<CanvasLayerEditor> canvasLayerEditor_;

    /** ImGui scene view 用 GPU texture ID */
    uint64_t sceneImGuiTextureId_ = 0;

    /** SceneView 表示スケール */
    float sceneViewScale_ = 1.0f;

    /** Canvas名。DebugUIウィンドウ名・プリセットパスの一意化に使う */
    std::string name_;

public:
    /** @brief 初期化
     * @param _adapter DirectX adapter
     * @param _srv SRV manager
     * @param _debug Debug UI
     * @param _name Canvas名。ウィンドウ名・プリセットパスの一意化に使う
     */
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv, const GESTD::ReferencePtr<DebugUI>& _debug, const std::string& _name);

    /** @brief PostEffect factory を設定 */
    void SetFactory(GESTD::ReferencePtr<PostEffectFactory> _factory);

    /** @brief PostEffect factory を取得 */
    GESTD::ReferencePtr<PostEffectFactory> GetFactory() const { return factory_; }

    /** @brief Canvas名を取得 */
    const std::string& GetName() const { return name_; }

    /** @brief Overlay（合成後の画面に適用する常時構成チェーン）を取得。Canvas Editorからの編集用 */
    PostEffectChain* GetOverlayChain() const { return chain_.get(); }

    /** @brief 常時構成（型＋baseParameters）を"./Assets/Data/PostEffect/Canvases/<Canvas名>.json"から読み込む */
    void LoadPermanentConfig();

    /** @brief 常時構成を"./Assets/Data/PostEffect/Canvases/<Canvas名>.json"へ保存する */
    void SavePermanentConfig() const;

    /** @brief エフェクトを追加
     * @param _effect 追加するエフェクト
     */
    void Add(std::unique_ptr<IPostEffect> _effect);

    /** @brief Canvasを追加する（Z順は自動採番） */
    Canvas* AddCanvas(const std::string& _name);

    /** @brief Canvasを最前面グループに追加する */
    Canvas* AddCanvasTop(const std::string& _name);

    /** @brief Canvasを削除する */
    void RemoveCanvas(const std::string& _name);

    /** @brief 名前からCanvasを取得する
     * 該当Canvasが存在しなくても、保存済みの常時構成Json（"./Assets/Data/PostEffect/Canvases/<_name>.json"）が
     * 存在する場合は自動でCanvasを生成し、その設定を読み込んだ上で返す
     */
    Canvas* GetCanvas(const std::string& _name);

    /** @brief 全Canvasを Z順（昇順）で取得する（Editor等の一覧表示用） */
    std::vector<std::pair<CanvasLayer::ZOrder, Canvas*>> GetCanvases() const;

    /** @brief 各Canvasが自分のタスクキューを消化し、自分のRTへ描画する */
    void DrawObjects();

    /** @brief 各Canvasの常時構成PostEffectを適用する */
    void ApplyPostEffects();

    /** @brief Canvasを合成し、ワンショット演出を適用する（自分のRTへ描画するだけで、スワップチェーンへはまだ描かない） */
    void DrawCanvases();

    /** @brief DrawCanvases()の最終結果を、現在バインドされている描画先（通常はスワップチェーン）へ描画する */
    void Draw() const;

    /** @brief エフェクトの有効/無効を切り替える
     * @param _type エフェクトタイプ名
     * @param _enable 有効フラグ
     */
    void SetActive(const std::string& _type, bool _enable);

    /** @brief デバッグ UI を表示 */
    void Debug();

    /** @brief preset を適用する
     * 対象エフェクトは一時インスタンスとして生成され、再生完了後に破棄される
     * @param _presetName preset 名
     * @param _onComplete 完了時コールバック
     */
    void ApplyPreset(const std::string& _presetName, std::function<void()> _onComplete = nullptr);

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

    /** @brief Canvas Editor を開く */
    void OpenCanvasEditor();

    /** @brief 常時構成のエフェクトを取得する。無ければ生成する
     * @param _type effect type
     * @return effect instance。失敗時は nullptr
     */
    IPostEffect* Create(const std::string& _type);

    /** @brief ワンショット演出用の一時エフェクトを生成する
     * @param _type effect type
     * @return 生成した一時エフェクトのid。失敗時は0
     */
    uint32_t CreateTemporary(const std::string& _type);

    /** @brief idから一時エフェクトを取得する
     * @param _id CreateTemporaryが返したid
     * @return effect instance。見つからない場合は nullptr
     */
    IPostEffect* GetTemporary(uint32_t _id);

    /** @brief 一時エフェクトを破棄する
     * @param _id CreateTemporaryが返したid
     */
    void RemoveTemporary(uint32_t _id);

    /** @brief render texture をリサイズする */
    void ResizeRenderTextures();

    /** @brief SceneView が表示中かどうか */
    bool IsSceneViewActive();

    /** @brief SceneView の表示スケールを取得 */
    float GetSceneViewScale() const { return sceneViewScale_; }

private:
    /** @brief "./Assets/Data/PostEffect/<Canvas名>/presets.json"のパスを取得 */
    std::string PresetsFilePath() const { return "./Assets/Data/PostEffect/" + name_ + "/presets.json"; }
}; // class PostProcessExecutor

#endif // PostProcessExecutor_HPP_
