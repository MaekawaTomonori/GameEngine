#include "Particle.hpp"

#include "DebugUIWidgets.hpp"
#include "Utils.hpp"
#include "imgui_internal.h"
#include "Math/MathUtils.hpp"

void Particle::Initialize(float _duration) {
    uuid_ = Utils::GenerateUniqueId();
    duration_ = _duration;
}

void Particle::Update() {
    const float progress = duration_ > 0.f ? now_ / duration_ : 0.f;

    color_ = EvaluateGradient(colorKeys_, progress, color_);
    scale_ = EvaluateGradient(sizeKeys_, progress, scale_);

    if (update_) {
        update_(progress, origin_, position_, velocity_, color_);
    }

    static constexpr float DT = 1.f / 60.f;
    now_ += DT;
    position_ += velocity_ * DT;
    rotation_ += rotationVelocity_ * DT;
}

void Particle::Debug() {
    ImGui::PushID(uuid_.c_str());
    if (ImGui::TreeNode(uuid_.c_str())) {
        DebugUIWidgets::DragFloat3("Position", &position_.x, 0.1f);
        DebugUIWidgets::DragFloat3("Scale", &scale_.x, 0.1f);
        DebugUIWidgets::DragFloat3("Rotation", &rotation_.x, 0.01f);
        DebugUIWidgets::DragFloat3("RotVelocity", &rotationVelocity_.x, 0.01f);
        DebugUIWidgets::ColorEdit4("Color", &color_.x);
        DebugUIWidgets::DragFloat("Duration", &duration_, 0.01f, 0.f, 100.f);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

bool Particle::IsDead() const {
    return duration_ <= now_;
}

Vector3 Particle::GetPosition() const {
    return position_;
}

Vector3 Particle::GetScale() const {
    return scale_;
}

Vector3 Particle::GetRotation() const {
    return rotation_;
}

Vector4 Particle::GetColor() const {
    return color_;
}

Particle& Particle::SetOrigin(const Vector3& _origin) {
    origin_ = _origin;
    return *this;
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

Particle& Particle::SetColorKeys(const std::vector<GradientKey<Vector4>>& _keys) {
    colorKeys_ = _keys;
    return *this;
}

Particle& Particle::SetSizeKeys(const std::vector<GradientKey<Vector3>>& _keys) {
    sizeKeys_ = _keys;
    return *this;
}

Particle& Particle::SetVelocity(const Vector3& _velocity) {
    velocity_ = _velocity;
    return *this;
}

Particle& Particle::SetRotation(const Vector3& _rotation) {
    rotation_ = _rotation;
    return *this;
}

Particle& Particle::SetRotationVelocity(const Vector3& _rotationVelocity) {
    rotationVelocity_ = _rotationVelocity;
    return *this;
}

Particle& Particle::RandomizePosition(const Vector3& _min, const Vector3& _max) {
    position_ = Vector3{
        MathUtils::Random(_min.x, _max.x),
        MathUtils::Random(_min.y, _max.y),
        MathUtils::Random(_min.z, _max.z)
    };
    return *this;
}

Particle& Particle::RandomizeScale(const Vector3& _min, const Vector3& _max) {
    scale_ = Vector3{
        MathUtils::Random(_min.x, _max.x),
        MathUtils::Random(_min.y, _max.y),
        MathUtils::Random(_min.z, _max.z)
    };
    return *this;
}

Particle& Particle::RandomizeColor(const Vector4& _min, const Vector4& _max) {
    color_ = Vector4{
        MathUtils::Random(_min.x, _max.x),
        MathUtils::Random(_min.y, _max.y),
        MathUtils::Random(_min.z, _max.z),
        MathUtils::Random(_min.w, _max.w)
    };
    return *this;
}

Particle& Particle::RandomizeVelocity(const Vector3& _min, const Vector3& _max) {
    velocity_ = Vector3{
        MathUtils::Random(_min.x, _max.x),
        MathUtils::Random(_min.y, _max.y),
        MathUtils::Random(_min.z, _max.z)
    };
    return *this;
}

Particle& Particle::SetUpdateFunction(const std::function<void(float, const Vector3&, Vector3&, Vector3&, Vector4&)>& _func) {
    update_ = _func;
    return *this;
}
