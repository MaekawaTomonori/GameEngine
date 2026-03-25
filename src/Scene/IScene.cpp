#include "IScene.hpp"

#include "SceneSwitcher.hpp"

bool IScene::IsProgress() const {
    return progress_;
}

void IScene::Awake() {
    progress_ = true;
    OnEnable();
}

void IScene::Setup(SceneSwitcher* _switcher) {
    switcher_ = _switcher;
}

void IScene::SetName(const std::string& _name) {
    name_ = _name;
}

void IScene::Change() {
	if (!switcher_) return;
    if (next_.empty()) return;
    switcher_->Change(next_);
	progress_ = false;
}

void IScene::PlayTransition(Transition::Type _outType, Transition::Type _inType, std::function<void()> _onMidpoint) {
    if (!switcher_) return;
    switcher_->PlayTransition(_outType, _inType, std::move(_onMidpoint));
}

void IScene::PlayTransition(Transition::Type _type, std::function<void()> _onMidpoint) {
    if (!switcher_) return;
    switcher_->PlayTransition(_type, std::move(_onMidpoint));
}

PostProcessExecutor* IScene::PostEffect() const {
    return switcher_->GetContext().ppe;
}

ParticleSystem* IScene::Particle() const {
    return switcher_->GetContext().particle;
}
