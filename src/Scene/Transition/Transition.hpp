#ifndef Transition_HPP_
#define Transition_HPP_
#include "ITransitionEffect.hpp"
#include <memory>

class Transition {
public:
    enum class Type {
        None,
        Fade,
        Slide,
    };

private:
    std::unique_ptr<ITransitionEffect> effect_;
    Type type_ = Type::None;
    float defaultDuration_ = 1.0f;

public:
    Transition() = default;
    ~Transition() = default;

    void Initialize();
    void Update();
    void Draw();

    void Awake(Type _type, ITransitionEffect::State _state);
    void Awake(Type _type, ITransitionEffect::State _state, float _duration);

    bool InProgress() const;

    void SetDefaultDuration(float _duration);

private:
    void CreateEffect(Type _type);

}; // class Transition

#endif // Transition_HPP_
