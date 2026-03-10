#ifndef Fade_HPP_
#define Fade_HPP_
#include "ITransitionEffect.hpp"
#include <memory>

class Sprite;

class Fade : public ITransitionEffect {
    float duration_ = 1.0f;
    float time_ = 0.0f;
    float speed_ = 1.0f / 60.0f;
    float alpha_ = 0.0f;

    State state_ = State::None;

    std::unique_ptr<Sprite> sprite_;

public:
    Fade();
    ~Fade() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    void Start(State _state, float _duration) override;
    void Stop() override;

    bool IsFinished() const override;
    State GetCurrentState() const override { return state_; }
    float GetProgress() const override { return alpha_; }

}; // class Fade

#endif // Fade_HPP_
