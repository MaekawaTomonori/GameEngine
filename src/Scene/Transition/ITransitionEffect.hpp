#ifndef ITransitionEffect_HPP_
#define ITransitionEffect_HPP_

class ITransitionEffect {
public:
    enum class State {
        None,
        In,
        Out,
    };

    virtual ~ITransitionEffect() = default;

    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;

    virtual void Start(State _state, float _duration) = 0;
    virtual void Stop() = 0;

    virtual bool IsFinished() const = 0;

}; // class ITransitionEffect

#endif // ITransitionEffect_HPP_
