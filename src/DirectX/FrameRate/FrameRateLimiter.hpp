#ifndef FrameRateLimiter_HPP_
#define FrameRateLimiter_HPP_
#include <chrono>
#include <cstdint>

class FrameRateLimiter{
    uint16_t maxFps_;
    std::chrono::steady_clock::time_point reference_;
    bool vsyncEnabled_;

    std::chrono::steady_clock::time_point pre_;

public:
    explicit FrameRateLimiter(uint16_t _maxFps, bool _useVsync = true);
    void WaitForNextFrame();

    float GetCurrentFps() const;
    float GetMaxFps() const;
};

#endif // FrameRateLimiter_HPP_
