#include "Particle.hpp"

#include "Utils.hpp"
#include "imgui_internal.h"

void Particle::Initialize(float _duration) {
    uuid_ = Utils::GenerateUniqueId();
    duration_ = _duration;
}

void Particle::Update() {
    static constexpr float DT = 1.f / 60.f;
    duration_ -= DT;

    if (update_) {
        position_ += update_();
        return;
    }


}

void Particle::Debug() {
    ImGui::PushID(uuid_.c_str());
    if (ImGui::TreeNode(uuid_.c_str())) {
        ImGui::DragFloat3("Position", &position_.x, 0.1f);
        ImGui::DragFloat3("Scale", &scale_.x, 0.1f);
        ImGui::ColorEdit4("Color", &color_.x);
        ImGui::DragFloat("Duration", &duration_, 0.01f, 0.f, 100.f);
        ImGui::TreePop();
    }
    ImGui::PopID();
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

Particle& Particle::SetPosition(const Vector3& _position) {
    position_ = _position;
    return *this;
}

Particle& Particle::SetScale(const Vector3& _scale) {
    scale_ = _scale;
    return *this;
}

Particle& Particle::SetColor(const Vector4& _color) {
    color_ = _color;
    return *this;
}

Particle& Particle::SetUpdateFunction(const std::function<Vector3()>& _func) {
    update_ = _func;
    return *this;
}
