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

    bool awakePending_ = false;
    Type pendingType_ = Type::None;
    ITransitionEffect::State pendingState_ = ITransitionEffect::State::None;

public:
    Transition() = default;
    ~Transition() = default;

    void Initialize();
    void Update();
    void Draw();

    void Awake(Type _type, ITransitionEffect::State _state);
    void Awake(Type _type, ITransitionEffect::State _state, float _duration);

    bool InProgress();

    void SetDefaultDuration(float _duration);

    /** @brief ImGui上でトランジション状態を表示（ウィンドウ内に埋め込む形で呼ぶ）
     **/
    void Debug();

    Type GetType() const { return type_; }
    ITransitionEffect::State GetCurrentState() const;
    float GetProgress() const;

private:
    void CreateEffect(Type _type);

}; // class Transition

#endif // Transition_HPP_
