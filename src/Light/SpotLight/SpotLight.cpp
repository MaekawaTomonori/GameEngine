#include "SpotLight.h"

#include "Pattern/Singleton.hpp"
#include "imgui.h"
#include "Math/MathUtils.hpp"
#include "src/Json/Json.hpp"
#include "vendor/MagicEnum/magic_enum.hpp"

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
    uuid_ = uuid;
    light_ = sl;
}

void RawSpotLight::Save(std::string _path) {
    Json* json = Singleton<Json>::GetInstance();

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

void RawSpotLight::ImGuiSetting() {
    ImGui::PushID(uuid_.c_str());
    if (ImGui::TreeNode(uuid_.c_str())){
        ImGui::ColorEdit4("Color", &light_.color.x);
        ImGui::DragFloat3("Position", &light_.position.x, 0.1f);
        ImGui::DragFloat3("Direction", &light_.direction.x, 0.1f);
        ImGui::DragFloat("Distance", &light_.distance, 0.1f, 0.f);
        ImGui::DragFloat("Intensity", &light_.intensity, 0.01f, 0.f, 10.f);
        ImGui::DragFloat("decay", &light_.decay, 0.01f, 0.f);
        ImGui::DragFloat("cosAngle", &light_.cosAngle, 0.01f, light_.falloffStart);
        ImGui::DragFloat("falloffStart", &light_.falloffStart, 0.01f, 0.f);

	    if (ImGui::Button("Delete")){
	        enable_ = false;
	    }
        ImGui::TreePop();
    }
    ImGui::PopID();

    if ((MathUtils::F_PI * 2.f) <= light_.cosAngle){
        light_.cosAngle -= MathUtils::F_PI * 2.f;
    }
    if ((MathUtils::F_PI * 2.f) <= light_.falloffStart){
        light_.falloffStart -= MathUtils::F_PI * 2.f;
    }

    if (light_.falloffStart < light_.cosAngle){
        light_.falloffStart = light_.cosAngle + std::cos(MathUtils::F_PI / 10.f);
    }

    light_.direction = light_.direction.Normalize();
}

void RawSpotLight::FollowRef() {
    if (ref_.has_value()) {
        light_.position = ref_.value();
    }
}
