#ifndef IScene_HPP_
#define IScene_HPP_
#include <string>

#include "src/ParticleSystem/ParticleSystem.hpp"
#include "src/Scene/Transition/Transition.hpp"

class SceneSwitcher;
class DebugUI;
class PostProcessExecutor;

/// <summary>
/// ゲームシーンの基底インターフェース
/// シーンのライフサイクル管理とトランジションを提供
/// </summary>
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

    /// <summary>
    /// 初期化処理（純粋仮想関数）
    /// </summary>
    virtual void Initialize() = 0;

    /// <summary>
    /// 更新処理（純粋仮想関数）
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// 描画処理（純粋仮想関数）
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// 終了処理
    /// </summary>
    virtual void Finalize(){}

    /// <summary>
    /// シーンが進行中かを判定
    /// </summary>
    /// <returns>進行中の場合true</returns>
    bool IsProgress() const;

    /// <summary>
    /// シーンを起動
    /// </summary>
    void Awake();

    /// <summary>
    /// シーンのセットアップ
    /// </summary>
    /// <param name="_switcher">シーン切り替え</param>
    void Setup(SceneSwitcher* _switcher);

    /// <summary>
    /// シーン名を取得
    /// </summary>
    /// <returns>シーン名への参照</returns>
    const std::string& GetName() const { return name_; }

    /// <summary>
    /// 入場トランジションタイプを取得
    /// </summary>
    /// <returns>トランジションタイプ</returns>
    Transition::Type GetEntryTransition() const { return entryTransition_; }

    /// <summary>
    /// 退場トランジションタイプを取得
    /// </summary>
    /// <returns>トランジションタイプ</returns>
    Transition::Type GetExitTransition() const { return exitTransition_; }

protected:
    /// <summary>
    /// シーンをnext_に変更
    /// </summary>
    void Change();

    PostProcessExecutor* PostEffect() const;
    DebugUI* DebugUI() const;
    ParticleSystem* Particle() const;
}; // class IScene

#endif // IScene_HPP_
