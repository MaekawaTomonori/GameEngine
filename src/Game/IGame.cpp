#include "include/IGame.hpp"
#include "Factory/AbstractPostEffectFactory.hpp"

SceneSwitcher * IGame::GetSceneSwitcher() const {
    return scene_.get();
}

IGame::IGame(std::unique_ptr<AbstractSceneFactory> _factory, const std::string &_scene) {
    scene_ = std::make_unique<SceneSwitcher>();
    if (_factory){
        scene_->SetFactory(std::move(_factory));
        scene_->Change(_scene);
    }
}

GameEngine::Config & IGame::GetCurrentConfig() {
    if (!config_) {
        config_ = std::make_unique<GameEngine::Config>();
    }
    return config_->Get();
}

void IGame::SetPostEffectFactory(std::unique_ptr<AbstractPostEffectFactory> _factory) {
    postEffectFactory_ = std::move(_factory);
}

AbstractPostEffectFactory* IGame::GetPostEffectFactory() const {
    return postEffectFactory_.get();
}
