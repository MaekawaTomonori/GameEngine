#include "include/IGame.hpp"
#include "Factory/AbstractPostEffectFactory.hpp"

IGame::IGame() {
    scene_ = std::make_unique<SceneSwitcher>();
}

IGame::~IGame() = default;

GESTD::ReferencePtr<SceneSwitcher> IGame::GetSceneSwitcher() const {
    return GESTD::ReferencePtr<SceneSwitcher>(scene_);
}

void IGame::SetPostEffectFactory(std::unique_ptr<AbstractPostEffectFactory> _factory) {
    postEffectFactory_ = std::move(_factory);
}

GESTD::ReferencePtr<AbstractPostEffectFactory> IGame::GetPostEffectFactory() const {
    return GESTD::ReferencePtr<AbstractPostEffectFactory>(postEffectFactory_);
}
