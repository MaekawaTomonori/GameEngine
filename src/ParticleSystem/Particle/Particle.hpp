#ifndef Particle_HPP_
#define Particle_HPP_
#include <functional>

#include "Math/Matrix.hpp"
#include "Math/Vector4.hpp"

class Particle {
    std::function<void()> update_;

    Vector3 position_{};
    Vector3 scale_{1.f, 1.f, 1.f};
    Vector4 color_{1.f, 1.f, 1.f, 1.f};
    float duration_ = 0.f;

public:
    void Initialize(float _duration);
    void Update();

    bool IsDead() const;

    Vector3 GetPosition() const;
    Vector3 GetScale() const;
    Vector4 GetColor() const;

private:

}; // class Particle

#endif // Particle_HPP_
