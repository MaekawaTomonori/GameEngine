#include "FrameRateLimiter.hpp"

#include <thread>

FrameRateLimiter::FrameRateLimiter(uint16_t _maxFps, bool _useVsync): maxFps_(maxFps), vsyncEnabled_(useVsync), reference_(std::chrono::steady_clock::now()){
}

void FrameRateLimiter::WaitForNextFrame() {
    const std::chrono::microseconds TargetFrameTime(static_cast<uint64_t>(1e6 / (maxFps_ + 5)));

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::chrono::microseconds elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

    if (elapsedTime < TargetFrameTime){
        while (std::chrono::steady_clock::now() - reference_ < TargetFrameTime) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    //Log::Send(Log::Level::DEBUG, "Frame Rate Limiter: Frame time: " + std::to_string(elapsedTime.count()) + " micro-sec");
    pre_ = reference_;
    reference_ = std::chrono::steady_clock::now();
}

float FrameRateLimiter::GetCurrentFps() const {
    return 1e6f / std::chrono::duration<float>(reference_-pre_).count();
}

float FrameRateLimiter::GetMaxFps() const {
    return static_cast<float>(maxFps_);
}
