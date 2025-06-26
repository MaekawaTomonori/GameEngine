#ifndef FrameRateLimiter_HPP_
#define FrameRateLimiter_HPP_
#include <chrono>
#include <cstdint>

class FrameRateLimiter{
    uint16_t maxFps_;
    std::chrono::steady_clock::time_point reference_;
    bool vsyncEnabled_;

public:
    explicit FrameRateLimiter(uint16_t maxFps, bool useVsync = true);

    void WaitForNextFrame();
};

#endif // FrameRateLimiter_HPP_
