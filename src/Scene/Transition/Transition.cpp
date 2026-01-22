#include "Transition.hpp"
#include "Fade.hpp"

void Transition::Initialize() {
    type_ = Type::None;
}

void Transition::Update() {
    if (!effect_) return;
    effect_->Update();
}

void Transition::Draw() {
    if (effect_) {
        effect_->Draw();
    }
}

void Transition::Awake(const Type _type, const ITransitionEffect::State _state) {
    if (_type==Type::None)return;
    Awake(_type, _state, defaultDuration_);
}

void Transition::Awake(const Type _type, const ITransitionEffect::State _state, const float _duration) {
    if (_type == Type::None){
        effect_.reset();
        return;
    }
    if (type_ != _type){
        type_ = _type;
        CreateEffect(_type);
        effect_->Initialize();
    }
    if (effect_) {
        effect_->Start(_state, _duration);
    }
}

bool Transition::InProgress() const {
    if (!effect_) {
        return false;
    }
    return !effect_->IsFinished();
}

void Transition::SetDefaultDuration(const float _duration) {
    defaultDuration_ = _duration;
}

void Transition::CreateEffect(const Type _type) {
    std::unique_ptr<ITransitionEffect> _effect = nullptr;
    switch (_type) {
        case Type::Fade:
            _effect = std::make_unique<Fade>();
            break;
        case Type::Slide:
            break;
        case Type::None:
            break;
        default: ;
    }
    if (effect_)effect_.reset();
    effect_ = std::move(_effect);
}
