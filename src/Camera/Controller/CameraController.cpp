#include "CameraController.hpp"

#include "DebugUI.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "src/Json/Json.hpp"

void CameraController::Initialize(float _ratio, DebugUI* _debug) {
    debug_ = debug;
    repository_ = std::make_unique<CameraRepository>();
    repository_->Initialize(ratio);

    Load();

    if (repository_->IsEmpty()) {
        Add("Default");
        SetActive("Default");
    }
}

void CameraController::Update() {
    Debug();
    if (activeCamera_) {
        activeCamera_->Update();
    }
}

Camera* CameraController::GetActive() const {
    return activeCamera_;
}

Camera* CameraController::Add(const std::string& _name) {
    return repository_->Add(name);
}

Camera* CameraController::SetActive(const std::string& _name) {
    if (repository_->IsEmpty()) {
        Utils::Alert("CameraController::SetActive: No cameras available.");
        return nullptr;
    }

    if (Camera* camera = repository_->Get(name)) {
        activeCamera_ = camera;
        return activeCamera_;
    }

    Utils::Alert("CameraController::SetActive: Camera not _found, setting to first camera.");
    if (!repository_->GetFirstName().empty()) {
        activeCamera_ = repository_->Get(repository_->GetFirstName());
    }

    return activeCamera_;
}

void CameraController::Debug() {
    debug_->RegisterCommand("CM", [&](){
        ImGui::Begin("Camera");

        if (ImGui::BeginTabBar("Camera")){
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
                    auto names = repository_->GetNames();
                    for (const auto& name : names){
                        if (ImGui::Button(name.c_str())){
                            SetActive(name);
                        }
                    }
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Active")) {
                if (activeCamera_) {
                    ImGui::Text("Active Camera: %s", activeCamera_->GetUniqueId().c_str());
                    activeCamera_->Debug();
                } else {
                    ImGui::Text("No active camera");
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    });
}

void CameraController::Load() {
    activeCamera_ = nullptr;
    repository_->LoadFromFile();

    if (!repository_->IsEmpty()) {
        SetActive(repository_->GetFirstName());
    }
}

void CameraController::Save() {
    repository_->SaveToFile();
}
