#include "Camera.hpp"

#include "DebugUI.hpp"
#include "Singleton.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "Math/MathUtils.hpp"

Camera::Camera() :uuid_(Utils::GenerateUniqueId()){
}

void Camera::Initialize(float _ratio) {
    transform_ = {
        {1, 1, 1},
        {0, 0, 0},
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

}
