#include "IScene.hpp"

#include "Pattern/Singleton.hpp"
#include "SceneSwitcher.hpp"
#include "src/Model/Common/ModelCommon.hpp"

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

void IScene::SetSky(const std::string& _textureKey) {
    if (!sky_) {
        sky_ = std::make_unique<Skybox>();
        sky_->Initialize(_textureKey);
    } else {
        sky_->SetTexture(_textureKey);
    }
    Singleton<ModelCommon>::GetInstance()->SetEnvironmentTexture(_textureKey);
}

void IScene::UpdateSky() {
    if (sky_) sky_->Update();
}

void IScene::DrawSky() {
    if (sky_) sky_->Draw();
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

GESTD::ReferencePtr<PostProcessExecutor> IScene::PostEffect() const {
    return switcher_->GetContext().ppe;
}

GESTD::ReferencePtr<ParticleSystem> IScene::Particle() const {
    return switcher_->GetContext().particle;
}
