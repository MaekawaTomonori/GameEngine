#include "Time.hpp"

#include "Pattern/Singleton.hpp"

float Time::GetDeltaTime() {
    return Instance().deltaTime_;
}

float Time::GetTimeScale() {
    return Instance().timeScale_;
}

void Time::SetTimeScale(float _scale, float _duration) {
    Instance().SetTimeScaleImpl(_scale, _duration);
}

void Time::Tick(float _realDeltaSeconds) {
    Instance().TickImpl(_realDeltaSeconds);
}

Time& Time::Instance() {
    return *Singleton<Time>::GetInstance();
}

void Time::TickImpl(float _realDeltaSeconds) {
    if (scaleDuration_ > 0.f) {
        scaleDuration_ -= _realDeltaSeconds;
        if (scaleDuration_ <= 0.f) {
            scaleDuration_ = 0.f;
            timeScale_ = 1.f;
        }
    }

    deltaTime_ = _realDeltaSeconds * timeScale_;
}

void Time::SetTimeScaleImpl(float _scale, float _duration) {
    timeScale_ = _scale;
    scaleDuration_ = _duration;
}
