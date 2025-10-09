#ifndef SceneSwitcher_HPP_
#define SceneSwitcher_HPP_
#include <memory>
#include <string>

#include "DebugUI.hpp"
#include "IScene.hpp"
#include "Factory/AbstractSceneFactory.hpp"

class SceneSwitcher {
	std::unique_ptr<AbstractSceneFactory> factory_;

    DebugUI* debug_ = nullptr;

    std::unique_ptr<IScene> scene_;
    std::unique_ptr<IScene> next_;

public:
	void Update();
    void Draw();

    void SetFactory(std::unique_ptr<AbstractSceneFactory> _factory);
    void Change(const std::string& _name);

    void Debug();
    void SetDebugUI(DebugUI* _debug) { debug_ = _debug; }
}; // class SceneSwitcher

#endif // SceneSwitcher_HPP_
