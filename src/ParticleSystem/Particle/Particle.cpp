#include "Particle.hpp"

void Particle::Initialize(float _duration) {
    duration_ = _duration;
}

void Particle::Update() {
    static constexpr float DT = 1.f / 60.f;
    duration_ -= DT;

    if (update_) {
        update_();
        return;
    }


}

bool Particle::IsDead() const {
    return duration_ <= 0.f;
}

Vector3 Particle::GetPosition() const {
    return position_;
}

Vector3 Particle::GetScale() const {
    return scale_;
}

Vector4 Particle::GetColor() const {
    return color_;
}
