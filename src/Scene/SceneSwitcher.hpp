#ifndef SceneSwitcher_HPP_
#define SceneSwitcher_HPP_
#include <memory>
#include <string>

#include "DebugUI.hpp"
#include "IScene.hpp"
#include "Factory/AbstractSceneFactory.hpp"

class Transition;

class SceneSwitcher {
	std::unique_ptr<AbstractSceneFactory> factory_;

    PostProcessExecutor* ppe_ = nullptr;
    DebugUI* debug_ = nullptr;

    std::unique_ptr<IScene> scene_;
    std::unique_ptr<IScene> next_;

    std::unique_ptr<Transition> transition_;

public:
    void Setup(PostProcessExecutor* _ppe, DebugUI* _debug);
	void Update();
    void Draw();

    void SetFactory(std::unique_ptr<AbstractSceneFactory> _factory);
    void Change(const std::string& _name);

    void Debug();
}; // class SceneSwitcher

#endif // SceneSwitcher_HPP_
