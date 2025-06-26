#ifndef SceneSwitcher_HPP_
#define SceneSwitcher_HPP_
#include <memory>
#include <string>

#include "IScene.hpp"
#include "Factory/AbstractSceneFactory.hpp"

class SceneSwitcher {
	std::unique_ptr<AbstractSceneFactory> factory_;

    std::unique_ptr<IScene> scene_;
    std::unique_ptr<IScene> next_;

public:
	void Update();
    void Draw();

    void SetFactory(std::unique_ptr<AbstractSceneFactory> _factory);
    void Change(const std::string& _name);
}; // class SceneSwitcher

#endif // SceneSwitcher_HPP_
