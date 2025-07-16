#include "Camera.hpp"

#include "DebugUI.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "Math/MathUtils.hpp"

Camera::Camera() :uuid_(Utils::GenerateUniqueId()){
}

void Camera::Initialize(float _ratio) {
    transform_ = {
        {1, 1, 1},
        Vector3{0, 0, 0},
        {0, 0, -5.f}
    };
    aspectRatio_ = _ratio;
}

void Camera::Update() {
    matrix_ = MathUtils::Matrix::MakeAffineMatrix(transform_);
    view_ = matrix_.Inverse();
    projection_ = MathUtils::Matrix::MakePerspectiveFovMatrix(fov_, aspectRatio_, near_, far_);
}

void Camera::Debug() {
    //Manager's Command { -> this -> next -> ...}
    // Inside collapsing header
    ImGui::Text("Scale: %f, %f, %f", transform_.scale.x, transform_.scale.y, transform_.scale.z);
    ImGui::DragFloat3("Rotate", &std::get<Vector3>(transform_.rotate).x, 0.1f);
    ImGui::DragFloat3("Translate", &transform_.translate.x, 0.1f);
    ImGui::DragFloat("FOV", &fov_, 0.01f, 0.01f, 10.f);
    ImGui::DragFloat("Aspect Ratio", &aspectRatio_, 0.01f, 0.01f, 10.0f);
    ImGui::DragFloat("Near Plane", &near_, 0.01f, 0.01f, 100.0f);
    ImGui::DragFloat("Far Plane", &far_, near_, 100.0f, 10000.0f);
}

std::string Camera::GetUniqueId() {
    return uuid_;
}

Matrix4x4 Camera::GetViewProjection() const {
    return view_ * projection_;
}

CameraForGpu Camera::GetCameraForGpu() const {
    return { transform_.translate };
}
