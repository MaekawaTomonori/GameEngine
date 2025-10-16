#define NOMINMAX
#include "Fade.hpp"
#include "Sprite.hpp"
#include "src/Config/Config.hpp"
#include <algorithm>

Fade::Fade() {
    sprite_ = std::make_unique<Sprite>();
}

void Fade::Initialize() {
    sprite_->Initialize("BlackFilter.png");

    auto* config = GameEngine::Config::Default();
    float width = static_cast<float>(config->GetWidth());
    float height = static_cast<float>(config->GetHeight());

    sprite_->SetSize({width, height});
    sprite_->SetPosition({width * 0.5f, height * 0.5f});
    sprite_->SetColor({0.0f, 0.0f, 0.0f, alpha_});
}

void Fade::Update() {
    if (state_ == State::None) {
        return;
    }

    time_ -= speed_;
    if (time_ <= 0.f) {
        time_ = 0.f;
        state_ = State::None;
    }

    float percentage = std::clamp(time_ / duration_, 0.0f, 1.0f);
    switch (state_) {
        case State::None:
            break;

        case State::Out:
            alpha_ = 1.0f - percentage;
            break;

        case State::In:
            alpha_ = percentage;
            break;
    }

    alpha_ = std::clamp(alpha_, 0.0f, 1.0f);

    sprite_->SetColor({0.0f, 0.0f, 0.0f, alpha_});
    sprite_->Update();
}

void Fade::Draw() {
    if (state_ == State::None) {
        return;
    }
    sprite_->Draw();
}

void Fade::Start(State _state, float _duration) {
    state_ = _state;
    duration_ = _duration;
    time_ = duration_;

    if (_state == State::In) {
        alpha_ = 1.0f;
    } else if (_state == State::Out) {
        alpha_ = 0.0f;
    }
}

void Fade::Stop() {
    state_ = State::None;
}

bool Fade::IsFinished() const {
    switch (state_) {
        case State::In:
        case State::Out:
            return time_ <= 0.0f;
        case State::None:
            return true;
    }
    return true;
}
