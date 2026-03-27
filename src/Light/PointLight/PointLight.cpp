#include "PointLight.h"
#include "Pattern/Singleton.hpp"
#include "imgui.h"
#include "src/Json/JsonParams.hpp"
#include "MagicEnum/magic_enum.hpp"

void RawPointLight::DefaultSetting() {
    type_ = LightType::Point;
	light_.color = {1, 1, 1, 1};
	light_.position = {0, 2, 0};
	light_.intensity = 0.f;
	light_.radius = 10;
	light_.decay = 1;
}

void RawPointLight::Set(const std::string& _uuid ,const PointLight& _pl) {
    uuid_ = _uuid;
    light_ = _pl;
}

void RawPointLight::Save(const std::string& _path) {
	JsonParams* json = Singleton<JsonParams>::GetInstance();
	json->SetValue(_path, uuid_, "type", magic_enum::enum_integer(type_));
	json->SetValue(_path, uuid_, "color", light_.color);
	json->SetValue(_path, uuid_, "position", light_.position);
	json->SetValue(_path, uuid_, "intensity", light_.intensity);
	json->SetValue(_path, uuid_, "radius", light_.radius);
	json->SetValue(_path, uuid_, "decay", light_.decay);
}

void RawPointLight::ImGuiSetting(int _index) {
    ImGui::PushID(uuid_.c_str());
    const std::string label = "Point " + std::to_string(_index);
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::ColorEdit4("Color", &light_.color.x);

        if (HasRef()) {
            ImGui::BeginDisabled();
            ImGui::DragFloat3("Position (ref)", &light_.position.x, 0.1f);
            ImGui::EndDisabled();
        } else {
            ImGui::DragFloat3("Position", &light_.position.x, 0.1f);
        }

        ImGui::DragFloat("Intensity", &light_.intensity, 0.01f, 0.f, 10.f);
        ImGui::DragFloat("Radius",    &light_.radius,    0.1f,  0.f);
        ImGui::DragFloat("Decay",     &light_.decay,     0.01f, 0.f);
        if (ImGui::Button("Delete")) { enable_ = false; }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void RawPointLight::FollowRef() {
    if (ref_.has_value()) {
        light_.position = ref_.value();
    }
}
