#ifndef IScene_HPP_
#define IScene_HPP_
#include <memory>
#include <string>

#include "SceneSwitcher.hpp"


class IScene {
    SceneSwitcher* switcher_ = nullptr;;
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

    void IsProgress() const;
    void Awake();

protected:
    void ChangeScene();
}; // class IScene

#endif // IScene_HPP_
