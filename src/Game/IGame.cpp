#include "include/IGame.hpp"
#include "Factory/AbstractPostEffectFactory.hpp"

IGame::IGame() {
    scene_ = std::make_unique<SceneSwitcher>();
}

IGame::~IGame() = default;

SceneSwitcher * IGame::GetSceneSwitcher() const {
    return scene_.get();
}

void IGame::SetPostEffectFactory(std::unique_ptr<AbstractPostEffectFactory> _factory) {
    postEffectFactory_ = std::move(_factory);
}

AbstractPostEffectFactory* IGame::GetPostEffectFactory() const {
    return postEffectFactory_.get();
}
