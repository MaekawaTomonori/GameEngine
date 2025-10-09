#ifndef IGame_HPP_
#define IGame_HPP_

#include <memory>

#include "src/Scene/SceneSwitcher.hpp"
#include "src/Config/Config.hpp"

class AbstractSceneFactory;
class PostProcessExecutor;

class IGame {
    std::unique_ptr<GameEngine::Config> config_;
    std::unique_ptr<SceneSwitcher> scene_;

protected:
    PostProcessExecutor* postProcessor_ = nullptr;

public:
    IGame(std::unique_ptr<AbstractSceneFactory> _factory, const std::string &_scene = "");
    virtual ~IGame() = default;

    GameEngine::Config& GetCurrentConfig();
    SceneSwitcher* GetSceneSwitcher() const;

    void SetPostProcessor(PostProcessExecutor* _post) { postProcessor_ = _post; }
    PostProcessExecutor* GetPostProcessor() const { return postProcessor_; }
}; // class IGame

#endif // IGame_HPP_
