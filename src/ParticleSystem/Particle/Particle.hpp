#ifndef Particle_HPP_
#define Particle_HPP_
#include <functional>
#include <string>
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"

class Particle {
    std::function<void(float, Vector3&, Vector4&)> update_;

    std::string uuid_;

    Vector3 position_{};
    Vector3 scale_{ 1.f, 1.f, 1.f };
    Vector3 velocity_ {};
    Vector4 color_{ 1.f, 1.f, 1.f, 1.f };
    float duration_ = 0.f;
    float now_ = 0.f;

public:
    void Initialize(float _duration);
    void Update();
    
    void Debug();

    bool IsDead() const;

    Vector3 GetPosition() const;
    Vector3 GetScale() const;
    Vector4 GetColor() const;

    Particle& SetPosition(const Vector3& _position);
    Particle& SetScale(const Vector3& _scale);
    Particle& SetVelocity(const Vector3& _velocity);
    Particle& SetColor(const Vector4& _color);

    Particle& RandomizePosition(const Vector3& _min = { -1.f, -1.f, -1.f }, const Vector3& _max = { 1.f, 1.f, 1.f });
    Particle& RandomizeScale(const Vector3& _min = { 0.1f, 0.1f, 0.1f }, const Vector3& _max = { 1.f, 1.f, 1.f });
    Particle& RandomizeVelocity(const Vector3& _min = { -1.f, -1.f, -1.f }, const Vector3& _max = { 1.f, 1.f, 1.f });
    Particle& RandomizeColor(const Vector4& _min = { 0.f, 0.f, 0.f, 0.f }, const Vector4& _max = { 1.f, 1.f, 1.f, 1.f });

    // float _t
    // Vector3& _velocity
    // Vector4& _color
    Particle& SetUpdateFunction(const std::function<void(float, Vector3&, Vector4&)>& _func);

private:

}; // class Particle

#endif // Particle_HPP_
