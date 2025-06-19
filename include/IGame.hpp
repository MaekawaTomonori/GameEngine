#ifndef IGame_HPP_
#define IGame_HPP_

#include <memory>

#include "src/Scene/SceneSwitcher.hpp"
#include "src/Config/Config.hpp"

class AbstractSceneFactory;

class IGame {
    std::unique_ptr<GameEngine::Config> config_;
    std::unique_ptr<SceneSwitcher> scene_;
public:
    IGame(std::unique_ptr<AbstractSceneFactory> _factory);
    virtual ~IGame() = default;

    GameEngine::Config& GetCurrentConfig();
    SceneSwitcher* GetSceneSwitcher() const;
}; // class IGame

#endif // IGame_HPP_
