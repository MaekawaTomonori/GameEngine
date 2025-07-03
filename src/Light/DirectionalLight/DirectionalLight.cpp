#include "DirectionalLight.h"

#include "Singleton.hpp"
#include "imgui.h"
#include "src/Json/Json.hpp"
#include "vendor/MagicEnum/magic_enum.hpp"

void RawDirectionalLight::DefaultSetting() {
    type_ = LightType::Directional;
    light_.color = {1, 1, 1, 1};
    light_.direction = {0, -1, 0};
    light_.intensity = 1.f;
}

void RawDirectionalLight::Save(std::string _path) {
    Json* json = Singleton<Json>::GetInstance();

    json->SetValue(_path, uuid_, "type", magic_enum::enum_integer(type_));
    json->SetValue(_path, uuid_, "color", light_.color);
    json->SetValue(_path, uuid_, "direction", light_.direction);
    json->SetValue(_path, uuid_, "intensity", light_.intensity);
}

void RawDirectionalLight::ImGuiSetting() {
    if (ImGui::TreeNode(uuid_.c_str())){
        ImGui::ColorEdit4("Color", &light_.color.x);
        ImGui::DragFloat3("Direction", &light_.direction.x, 0.1f);
        ImGui::DragFloat("Intensity", &light_.intensity, 0.01f, 0, 1);


        if (ImGui::Button("Delete")){
            enable_ = false;
        }
        ImGui::TreePop();
    }

    light_.direction.Normalize();
}
