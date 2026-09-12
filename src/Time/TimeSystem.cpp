#include "TimeSystem.hpp"

float TimeSystem::GetDeltaTime() const {
    return deltaTime_;
}

float TimeSystem::GetUnscaledDeltaTime() const {
    return kFixedDeltaTime;
}

float TimeSystem::GetTimeScale() const {
    return timeScale_;
}

void TimeSystem::SetTimeScale(float _scale, float _duration) {
    timeScale_     = _scale;
    scaleDuration_ = _duration;
}

void TimeSystem::Tick(float _realDeltaSeconds) {
    if (scaleDuration_ > 0.f) {
        scaleDuration_ -= _realDeltaSeconds;
        if (scaleDuration_ <= 0.f) {
            scaleDuration_ = 0.f;
            timeScale_     = 1.f;
        }
    }

    deltaTime_ = kFixedDeltaTime * timeScale_;
}
