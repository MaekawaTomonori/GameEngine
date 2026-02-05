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

template<typename T>
void IGame::RegisterScene(const std::string& _name) {
    static_assert(std::is_base_of_v<IScene, T>, "T must be derived from IScene");
    scene_->RegisterScene(_name, [] { return std::make_unique<T>(); });
}
