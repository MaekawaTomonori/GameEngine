#ifndef IScene_HPP_
#define IScene_HPP_
#include <string>
#include "src/ParticleSystem/ParticleSystem.hpp"
#include "src/Scene/Transition/Transition.hpp"

class SceneSwitcher;
class DebugUI;
class PostProcessExecutor;

/** @brief ゲームシーンの基底インターフェース
 ** シーンのライフサイクル管理とトランジションを提供
 **/
class IScene {
    SceneSwitcher* switcher_ = nullptr;
    bool progress_ = false;

protected:
    std::string name_;
    std::string next_;

    Transition::Type entryTransition_ = Transition::Type::None;
    Transition::Type exitTransition_ = Transition::Type::None;

public:
    virtual ~IScene() = default;

    /** @brief 初期化処理（純粋仮想関数）
     **/
    virtual void Initialize() = 0;

    /** @brief 更新処理（純粋仮想関数）
     **/
    virtual void Update() = 0;

    /** @brief 描画処理（純粋仮想関数）
     **/
    virtual void Draw() = 0;

    /** @brief 終了処理
     **/
    virtual void Finalize() {}

    /** @brief シーンが進行中かを判定
     ** @return 進行中の場合true
     **/
    bool IsProgress() const;

    /** @brief シーンを起動
     **/
    void Awake();

    /** @brief シーンのセットアップ
     ** @param _switcher シーン切り替え
     **/
    void Setup(SceneSwitcher* _switcher);

    /** @brief シーン名を取得
     ** @return シーン名への参照
     **/
    const std::string& GetName() const { return name_; }

    /** @brief 入場トランジションタイプを取得
     ** @return トランジションタイプ
     **/
    Transition::Type GetEntryTransition() const { return entryTransition_; }

    /** @brief 退場トランジションタイプを取得
     ** @return トランジションタイプ
     **/
    Transition::Type GetExitTransition() const { return exitTransition_; }

    /** @brief Debug用 ImGuiはここで扱うようにする
     **/
    virtual void Debug() {}

protected:
    /** @brief シーンをnext_に変更
     **/
    void Change();

    PostProcessExecutor* PostEffect() const;
    ParticleSystem* Particle() const;
}; // class IScene

#endif // IScene_HPP_
