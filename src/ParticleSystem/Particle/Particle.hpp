#ifndef Particle_HPP_
#define Particle_HPP_
#include <functional>
#include <string>

#include "Math/Matrix.hpp"
#include "Math/Vector4.hpp"

class Particle {
    std::function<Vector3()> update_;

    std::string uuid_;

    Vector3 position_{};
    Vector3 scale_{ 1.f, 1.f, 1.f };
    Vector4 color_{ 1.f, 1.f, 1.f, 1.f };
    float duration_ = 0.f;

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
    Particle& SetColor(const Vector4& _color);

    Particle& SetUpdateFunction(const std::function<Vector3()>& _func);

private:

}; // class Particle

#endif // Particle_HPP_
