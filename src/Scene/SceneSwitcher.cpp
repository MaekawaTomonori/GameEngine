#include "SceneSwitcher.hpp"

#include "imgui.h"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Director/CameraDirector.hpp"
#include "src/Scene/Transition/Transition.hpp"

void SceneSwitcher::Setup(Context _context) {
    context_ = _context;

    transition_ = std::make_unique<Transition>();
    transition_->Initialize();
}

void SceneSwitcher::Update() {
    Debug();

    // Scene Change/ in Transition process
    // Transition in/out
    if (transition_->InProgress()){
        transition_->Update();
        return;
    }

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
        scene_->Setup(this);
        scene_->Initialize();

        transition_->Awake(scene_->GetEntryTransition(), ITransitionEffect::State::In, 1.f);
        return;
    }

    if (!scene_)return;

    if (!scene_->IsProgress()){
        scene_->Awake();
    }

    if (scene_->IsProgress()){
        scene_->Update();
    }
}

void SceneSwitcher::Draw() {
    if (scene_){
        scene_->Draw();
    }
    if (transition_) {
        transition_->Draw();
    }
}

void SceneSwitcher::Debug() {
    context_.debug->RegisterCommand("SceneSwitcher", [this]() {
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

    // Already Destroyed
    if (!scene_) return;

    // Scene Change Transition
    transition_->Awake(scene_->GetExitTransition(), ITransitionEffect::State::Out);
}
