#include "PointLight.h"

#include "Singleton.hpp"
#include "imgui.h"
#include "src/Json/Json.hpp"
#include "vendor/MagicEnum/magic_enum.hpp"


void RawPointLight::DefaultSetting() {
    type_ = LightType::Point;
	light_.color = {1, 1, 1, 1};
	light_.position = {0, 2, 0};
	light_.intensity = 0.f;
	light_.radius = 10;
	light_.decay = 1;
}

void RawPointLight::Set(const std::string& uuid ,const PointLight& pl) {
    uuid_ = uuid;
    light_ = pl;
}

void RawPointLight::Save(std::string _path) {
	Json* json = Singleton<Json>::GetInstance();

	json->SetValue(_path, uuid_, "type", magic_enum::enum_integer(type_));
	json->SetValue(_path, uuid_, "color", light_.color);
	json->SetValue(_path, uuid_, "position", light_.position);
	json->SetValue(_path, uuid_, "intensity", light_.intensity);
	json->SetValue(_path, uuid_, "radius", light_.radius);
	json->SetValue(_path, uuid_, "decay", light_.decay);
}

void RawPointLight::ImGuiSetting() {
 
	if (ImGui::TreeNode(uuid_.c_str())){
	    ImGui::ColorEdit4("Color", &light_.color.x);
	    ImGui::DragFloat3("Position", &light_.position.x, 0.1f);
	    ImGui::DragFloat("Intensity", &light_.intensity, 0.01f, 0, 1);
	    ImGui::DragFloat("radius", &light_.radius, 0.01f);
	    ImGui::DragFloat("decay", &light_.decay, 0.01f);

        if (ImGui::Button("Delete")){
            enable_ = false;
        }
        ImGui::TreePop();
    }
}
