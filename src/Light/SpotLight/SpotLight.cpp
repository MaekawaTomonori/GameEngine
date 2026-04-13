#include "SpotLight.h"

#include "Pattern/Singleton.hpp"
#include "imgui.h"
#include "Math/MathUtils.hpp"
#include "src/Json/JsonParams.hpp"
#include "MagicEnum/magic_enum.hpp"

void RawSpotLight::DefaultSetting() {
    type_ = LightType::Spot;
    light_.color = {1,1,1,1};
    light_.position = {2.f, 1.25f, 0.f};
    light_.intensity = 0.f;
    light_.direction = Vector3(-1.f, -1.f, 0).Normalize();
    light_.distance = 7.f;
    light_.decay = 2.f;
    light_.cosAngle = std::cos(MathUtils::F_PI / 3.f);
    light_.falloffStart = std::cos(MathUtils::F_PI / 4.f);
}

void RawSpotLight::Set(const std::string& _uuid, const SpotLight& _sl) {
    uuid_ = _uuid;
    light_ = _sl;
}

void RawSpotLight::Save(const std::string& _path) {
    const auto json = Singleton<JsonParams>::GetInstance();

    json->SetValue(_path, uuid_, "type", magic_enum::enum_integer(type_));
    json->SetValue(_path, uuid_, "color", light_.color);
    json->SetValue(_path, uuid_, "position", light_.position);
    json->SetValue(_path, uuid_, "intensity", light_.intensity);
    json->SetValue(_path, uuid_, "direction", light_.direction);
    json->SetValue(_path, uuid_, "distance", light_.distance);
    json->SetValue(_path, uuid_, "decay", light_.decay);
    json->SetValue(_path, uuid_, "cosAngle", light_.cosAngle);
    json->SetValue(_path, uuid_, "falloffStart", light_.falloffStart);
}

void RawSpotLight::ImGuiSetting(int _index) {
    ImGui::PushID(uuid_.c_str());
    const std::string label = "Spot " + std::to_string(_index);
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::ColorEdit4("Color", &light_.color.x);

        if (HasRef()) {
            ImGui::BeginDisabled();
            ImGui::DragFloat3("Position (ref)", &light_.position.x, 0.1f);
            ImGui::EndDisabled();
        } else {
            ImGui::DragFloat3("Position", &light_.position.x, 0.1f);
        }

        ImGui::DragFloat3("Direction", &light_.direction.x, 0.01f, -1.f, 1.f);
        ImGui::DragFloat("Intensity",  &light_.intensity, 0.01f, 0.f, 10.f);
        ImGui::DragFloat("Distance",   &light_.distance,  0.1f,  0.f);
        ImGui::DragFloat("Decay",      &light_.decay,     0.01f, 0.f);

        // cosAngle / falloffStart を度数表示に変換して操作
        float coneAngleDeg = std::acos(std::clamp(light_.cosAngle,     -1.f, 1.f)) * 180.f / MathUtils::F_PI;
        float falloffDeg   = std::acos(std::clamp(light_.falloffStart,  -1.f, 1.f)) * 180.f / MathUtils::F_PI;

        if (ImGui::DragFloat("Cone Angle (deg)",    &coneAngleDeg, 0.5f, 1.f, 89.f)) {
            light_.cosAngle = std::cos(coneAngleDeg * MathUtils::F_PI / 180.f);
        }
        if (ImGui::DragFloat("Falloff Start (deg)", &falloffDeg, 0.5f, 0.f, coneAngleDeg - 0.5f)) {
            light_.falloffStart = std::cos(falloffDeg * MathUtils::F_PI / 180.f);
        }

        if (ImGui::Button("Delete")) { enable_ = false; }
        ImGui::TreePop();
    }
    ImGui::PopID();

    light_.direction = light_.direction.Normalize();
}

void RawSpotLight::FollowRef() {
    if (ref_.has_value()) {
        light_.position = ref_.value();
    }
}
