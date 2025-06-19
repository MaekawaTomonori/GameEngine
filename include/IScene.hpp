#ifndef IScene_HPP_
#define IScene_HPP_
#include <string>

class SceneSwitcher;

class IScene {
    SceneSwitcher* switcher_ = nullptr;
    bool progress_ = false;
protected:
    std::string next_;

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
    void Awake(SceneSwitcher* _switcher);
    void SetSwitcher(SceneSwitcher* _switcher);

protected:
    void Change();
}; // class IScene

#endif // IScene_HPP_
