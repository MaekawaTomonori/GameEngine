#include "FrameRateLimiter.hpp"

#include <thread>

FrameRateLimiter::FrameRateLimiter(uint16_t maxFps, bool useVsync): maxFps_(maxFps), vsyncEnabled_(useVsync) {
    reference_ = std::chrono::steady_clock::now();
}

void FrameRateLimiter::WaitForNextFrame() {

    const std::chrono::microseconds TargetFrameTime(static_cast<uint64_t>(1e3 / (maxFps_ + 5)));

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::chrono::microseconds elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

    if (elapsedTime < TargetFrameTime){
        std::chrono::microseconds remaining = TargetFrameTime - elapsedTime;
        while (std::chrono::milliseconds(2) < remaining) {
            std::this_thread::sleep_for(remaining - std::chrono::milliseconds(2));
        }

        while (std::chrono::steady_clock::now() - reference_ < TargetFrameTime) {
            std::this_thread::yield();
        }
    }
    //Log::Send(Log::Level::DEBUG, "Frame Rate Limiter: Frame time: " + std::to_string(elapsedTime.count()) + " micro-sec");

    reference_ = std::chrono::steady_clock::now();
}
