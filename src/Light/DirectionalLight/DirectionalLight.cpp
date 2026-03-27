#include "DirectionalLight.h"

#include "Pattern/Singleton.hpp"
#include "imgui.h"
#include "src/Json/JsonParams.hpp"
#include "MagicEnum/magic_enum.hpp"

void RawDirectionalLight::DefaultSetting() {
    type_ = LightType::Directional;
    light_.color = {1, 1, 1, 1};
    light_.direction = {0, -1, 0};
    light_.intensity = 1.f;
}

void RawDirectionalLight::Save(const std::string& _path) {
    JsonParams* json = Singleton<JsonParams>::GetInstance();

    json->SetValue(_path, uuid_, "type", magic_enum::enum_integer(type_));
    json->SetValue(_path, uuid_, "color", light_.color);
    json->SetValue(_path, uuid_, "direction", light_.direction);
    json->SetValue(_path, uuid_, "intensity", light_.intensity);
}

void RawDirectionalLight::ImGuiSetting(int _index) {
    ImGui::PushID(uuid_.c_str());
    const std::string label = "Directional " + std::to_string(_index);
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::ColorEdit4("Color", &light_.color.x);
        ImGui::DragFloat3("Direction", &light_.direction.x, 0.01f, -1.f, 1.f);
        ImGui::DragFloat("Intensity", &light_.intensity, 0.01f, 0.f, 10.f);
        if (ImGui::Button("Delete")) { enable_ = false; }
        ImGui::TreePop();
    }
    ImGui::PopID();

    light_.direction = light_.direction.Normalize();
}

void RawDirectionalLight::FollowRef() { }
