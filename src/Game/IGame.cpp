#include "include/IGame.hpp"

SceneSwitcher * IGame::GetSceneSwitcher() const {
	return scene_.get();
}

IGame::IGame(std::unique_ptr<AbstractSceneFactory> _factory) {
	scene_ = std::make_unique<SceneSwitcher>();
    if (_factory){
        scene_->SetFactory(std::move(_factory));
    }
}

GameEngine::Config & IGame::GetCurrentConfig() {
    if (!config_) {
        config_ = std::make_unique<GameEngine::Config>();
    }
    return config_->Get();
}
