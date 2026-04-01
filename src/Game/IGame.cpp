#include "include/IGame.hpp"
#include "Factory/AbstractPostEffectFactory.hpp"

IGame::IGame() {
    scene_ = std::make_unique<SceneSwitcher>();
}

IGame::~IGame() = default;

GESTD::WeakPtr<SceneSwitcher> IGame::GetSceneSwitcher() const {
    return GESTD::WeakPtr<SceneSwitcher>(scene_);
}

void IGame::SetPostEffectFactory(std::unique_ptr<AbstractPostEffectFactory> _factory) {
    postEffectFactory_ = std::move(_factory);
}

GESTD::WeakPtr<AbstractPostEffectFactory> IGame::GetPostEffectFactory() const {
    return GESTD::WeakPtr<AbstractPostEffectFactory>(postEffectFactory_);
}
