#ifndef IScene_HPP_
#define IScene_HPP_
#include <string>

class SceneSwitcher;
class DebugUI;
class PostProcessExecutor;

class IScene {
    SceneSwitcher* switcher_ = nullptr;
    bool progress_ = false;

protected:
    std::string name_;
    std::string next_;

    PostProcessExecutor* postEffects_ = nullptr;
    DebugUI* debug_ = nullptr;

    // Fade Types
    // in
    // out
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

protected:
    void Change();
}; // class IScene

#endif // IScene_HPP_
