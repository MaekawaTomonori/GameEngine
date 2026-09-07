#include "include/IGame.hpp"

IGame::IGame() {
    scene_ = std::make_unique<SceneSwitcher>();
    postEffectFactory_ = std::make_unique<PostEffectFactory>();
}

IGame::~IGame() = default;

GESTD::ReferencePtr<SceneSwitcher> IGame::GetSceneSwitcher() const {
    return GESTD::ReferencePtr<SceneSwitcher>(scene_);
}

GESTD::ReferencePtr<PostEffectFactory> IGame::GetPostEffectFactory() const {
    return GESTD::ReferencePtr<PostEffectFactory>(postEffectFactory_);
}
