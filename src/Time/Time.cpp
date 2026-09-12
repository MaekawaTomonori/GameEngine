#include "Time.hpp"

#include "TimeSystem.hpp"
#include "Pattern/Singleton.hpp"

float Time::GetDeltaTime() const {
    return Singleton<TimeSystem>::GetInstance()->GetDeltaTime();
}

float Time::GetUnscaledDeltaTime() const {
    return Singleton<TimeSystem>::GetInstance()->GetUnscaledDeltaTime();
}

float Time::GetTimeScale() const {
    return Singleton<TimeSystem>::GetInstance()->GetTimeScale();
}

void Time::SetTimeScale(float _scale, float _duration) {
    Singleton<TimeSystem>::GetInstance()->SetTimeScale(_scale, _duration);
}
