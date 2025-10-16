#ifndef IScene_HPP_
#define IScene_HPP_
#include <string>
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

    PostProcessExecutor* postEffects_ = nullptr;
    DebugUI* debug_ = nullptr;

    Transition::Type entryTransition_ = Transition::Type::None;
    Transition::Type exitTransition_ = Transition::Type::None;
public:
    virtual ~IScene() = default;
    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void Finalize(){}

    bool IsProgress() const;
    void Awake();
    void Setup(SceneSwitcher* _switcher, PostProcessExecutor* _ppe, DebugUI* _debug) { switcher_ = _switcher; postEffects_ = _ppe; debug_ = _debug; }

    const std::string& GetName() const { return name_; }

    Transition::Type GetEntryTransition() const { return entryTransition_; }
    Transition::Type GetExitTransition() const { return exitTransition_; }

protected:
    void Change();
}; // class IScene

#endif // IScene_HPP_
