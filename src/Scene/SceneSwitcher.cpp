#include "SceneSwitcher.hpp"

#include "imgui.h"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Director/CameraDirector.hpp"

void SceneSwitcher::Setup(PostProcessExecutor* _ppe, DebugUI* _debug) {
    ppe_ = _ppe;
    debug_ = _debug;
}

void SceneSwitcher::Update() {
    Debug();
    // Scene Changer
    //if (changer_->InProgress()){
    //    changer_->Update();
    //    return;
    //}

    if (next_){
        if (scene_){
            scene_->Finalize();
            scene_.reset();
            scene_ = nullptr;
            Singleton<CameraDirector>::GetInstance()->Stop();
        }
        scene_ = std::move(next_);

        next_.reset();
        next_ = nullptr;
        scene_->Setup(this, ppe_, debug_);
        scene_->Initialize();
        scene_->Update();

        //changer_->Awake(scene_->GetEntryEffect(), ISceneEffect::State::In);
    }

    if (!scene_)return;

    // ChangeProcess End
    //if (changer_->InProgress())return;

    //if (!scene_->InProgress()){
        scene_->Awake();
    //}

    scene_->Update();
}

void SceneSwitcher::Draw() {
    if (scene_){ 
        scene_->Draw();
    }
}

void SceneSwitcher::Debug() {
    debug_->RegisterCommand("SceneSwitcher", [this]() {
        ImGui::Begin("SceneSwitcher");
        if (scene_) {
            ImGui::Text("Current Scene: %s", scene_->GetName().c_str());
        } else {
            ImGui::Text("No active scene");
        }
        ImGui::Separator();
        
        static char sceneName[128] = "";
        ImGui::InputText("Scene Name", sceneName, sizeof(sceneName));
        if (ImGui::Button("Change Scene")) {
            if (strlen(sceneName) > 0) {
                Change(sceneName);
            }
        }
        ImGui::End();
    });
}

void SceneSwitcher::SetFactory(std::unique_ptr<AbstractSceneFactory> _factory) {
    factory_ = std::move(_factory);
}

void SceneSwitcher::Change(const std::string &_name) {
    if (!factory_) return;

    // Already Registered
    if (next_) return;

    next_ = factory_->Create(_name);

    // out fade
    // Already Destroyed
    if (!scene_) return;

    // Fade
}
