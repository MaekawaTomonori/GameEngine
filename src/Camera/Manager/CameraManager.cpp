#include "CameraManager.hpp"

#include "DebugUI.hpp"
#include "Pattern/Singleton.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "src/Json/Json.hpp"

void CameraManager::Initialize(const float _ratio, DebugUI* _debug) {
    debug_ = _debug;
    ratio_ = _ratio;

    Load();

    if (cameras_.empty()) {
        Add("Default");
        SetActive("Default");
    }
}

void CameraManager::Update() {
    Debug();
    active_->Update();
}

Camera* CameraManager::GetActive() const {
    return active_;
}

Camera* CameraManager::Add(const std::string& _name) {
    if (cameras_.contains(_name))return cameras_[_name].get();

    if (_name.empty()) {
        return Add("noname" + std::to_string(nonameCount_++));
    }

    cameras_[_name] = std::make_unique<Camera>();
    cameras_[_name]->Initialize(ratio_);
    return cameras_[_name].get();
}

Camera* CameraManager::SetActive(const std::string& _name) {
    if (cameras_.empty()){
        Utils::Alert("CameraManager::SetActive: No cameras available.");
        return nullptr;
    }
    if (cameras_.contains(_name)){
        active_ = cameras_[_name].get();
        return active_;
    }
    Utils::Alert("CameraManager::SetActive: Camera not found, setting to first camera.");

    return active_;
}

void CameraManager::Debug() {
    debug_->RegisterCommand("CM", [&](){
        if (ImGui::Begin("Camera")){
            ImGui::BeginTabBar("Camera");
            if (ImGui::BeginTabItem("General")){
                if (ImGui::CollapsingHeader("Files")){
                    if (ImGui::Button("Load / Reload")){
                        Load();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Save")){
                        Save();
                    }
                }
                ImGui::Separator();
                if (ImGui::CollapsingHeader("List")){
                    for (const auto& name : cameras_ | std::views::keys){
                        if (ImGui::Button(name.c_str())){
                            SetActive(name);
                        }
                    }
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Active")) {
                ImGui::Text("Active Camera: %s", active_->GetUniqueId().c_str());
                active_->Debug();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
            ImGui::End();
        }
    });
}

void CameraManager::Load() {
    active_ = nullptr;
    cameras_.clear();

    Json* json = Singleton<Json>::GetInstance();
    if (!json->Load("Camera")) return;

    auto group = json->GetGroups("Camera");
    for (auto& [groupId, object] : group){
        Camera* camera = Add(groupId);
        camera->transform_ = {
            {1,1,1},
            std::get<Vector3>(object["Rotate"]),
            std::get<Vector3>(object["Position"])
        };
    }
    SetActive(cameras_.begin()->first);
}

void CameraManager::Save() {
    Json* json = Singleton<Json>::GetInstance();
    for (auto& [name, camera] : cameras_){
        json->SetValue("Camera", name, "Position", camera->transform_.translate);
        json->SetValue("Camera", name, "Rotate", std::get<Vector3>(camera->transform_.rotate));
    }
    json->Save("Camera");
}
