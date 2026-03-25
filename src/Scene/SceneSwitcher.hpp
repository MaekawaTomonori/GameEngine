#ifndef SceneSwitcher_HPP_
#define SceneSwitcher_HPP_
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "DebugUI.hpp"
#include "src/PostProcess/Editor/PostProcessPresetEditor.hpp"
#include "src/ParticleSystem/ParticleSystem.hpp"

#include "IScene.hpp"
#include "SceneFactory.hpp"

class Transition;
class FrameDebugger;

/** @brief シーン切り替え管理クラス
 * シーン遷移とトランジションエフェクトを管理
 */
class SceneSwitcher {
public:
    struct Context {
        PostProcessExecutor* ppe = nullptr;
        ParticleSystem* particle = nullptr;
        DebugUI* debug = nullptr;
        FrameDebugger* frame = nullptr;
    };

private:
    std::unique_ptr<SceneFactory> factory_;

    Context context_{};

    std::unique_ptr<IScene> scene_;
    std::unique_ptr<IScene> next_;

    std::unique_ptr<Transition> transition_;
    std::function<void()> midpointCallback_;
    Transition::Type intraOutType_ = Transition::Type::None;
    Transition::Type intraInType_  = Transition::Type::None;
    bool midpointPending_ = false;

    std::string homeScene_ = "sample";
    bool pendingBreak_ = false;

#ifdef _DEBUG
    enum class RunState { Stopped, Running, Paused };
    RunState debugRunState_ = RunState::Stopped;
#endif

public:
    SceneSwitcher();
    /** @brief セットアップ
     * @param _context コンテキスト
     */
    void Setup(const Context& _context);

    /** @brief 更新処理
     */
    void Update();

    /** @brief 描画処理
     */
    void Draw();

    /** @brief シーンを登録する
     * @param _name シーン名
     * @param _creator シーン生成関数
     */
    void RegisterScene(const std::string& _name, const std::function<std::unique_ptr<IScene>()>& _creator) const;

    /** @brief シーンを変更
     * @param _name シーン名
     */
    void Change(const std::string& _name);

    /** @brief トランジションを再生（Out→callback→In）
     * @param _outType アウト演出種別
     * @param _inType  イン演出種別
     * @param _onMidpoint 暗転しきった中点で呼ばれるコールバック（省略可）
     */
    void PlayTransition(Transition::Type _outType, Transition::Type _inType, std::function<void()> _onMidpoint = {});

    /** @brief トランジションを再生（Out/Inを同一タイプで実行）
     * @param _type 演出種別（Out・In 両方に適用）
     * @param _onMidpoint 暗転しきった中点で呼ばれるコールバック（省略可）
     */
    void PlayTransition(Transition::Type _type, std::function<void()> _onMidpoint = {});

    /** @brief デバッグ情報の表示
     */
    void Debug();

    const Context& GetContext() const;

private:
    /** @brief トランジションを即座にスキップし、コールバック状態をリセットする */
    void SkipTransition();

    void LocalDebugger();

}; // class SceneSwitcher

#endif // SceneSwitcher_HPP_
