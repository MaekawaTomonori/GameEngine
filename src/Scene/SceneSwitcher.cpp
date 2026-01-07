#include "SceneSwitcher.hpp"

#include "imgui.h"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Director/CameraDirector.hpp"
#include "src/Scene/Transition/Transition.hpp"

SceneSwitcher::SceneSwitcher() {
    factory_ = std::make_unique<SceneFactory>();
}

void SceneSwitcher::Setup(const Context& _context) {
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

        // データ送信用に1フレーム進める
        scene_->Update();

        transition_->Awake(scene_->GetEntryTransition(), ITransitionEffect::State::In, 1.f);
        return;
    }

    if (!scene_)return;

    if (!scene_->IsProgress()){
        scene_->Awake();
    }

    scene_->Update();
}

void SceneSwitcher::Draw() {
    if (scene_){
        scene_->Draw();
    }
    if (transition_) {
        transition_->Draw();
    }
}

void SceneSwitcher::RegisterScene(const std::string& _name, const std::function<std::unique_ptr<IScene>()>& _creator) const {
    if (!factory_) return;
    factory_->Register(_name, _creator);
}

void SceneSwitcher::Debug() {
    context_.debug->RegisterCommand("SceneSwitcher", [this]() {
        static int selectedIndex = 0;
        static std::string selectedScene;

        ImGui::Begin("SceneSwitcher");

        // 現在のシーン表示
        if (scene_) {
            ImGui::Text("Current Scene: %s", scene_->GetName().c_str());
        } else {
            ImGui::Text("No active scene");
        }

        ImGui::Separator();

        // 登録されているシーン一覧を取得
        if (factory_) {
            auto registeredScenes = factory_->GetRegisteredScenes();

            if (!registeredScenes.empty()) {
                // コンボボックス用のプレビュー文字列
                const char* previewValue = selectedIndex < registeredScenes.size()
                    ? registeredScenes[selectedIndex].c_str()
                    : "Select Scene";

                if (ImGui::BeginCombo("##Scene List", previewValue)) {
                    for (int i = 0; i < registeredScenes.size(); ++i) {
                        const bool isSelected = (selectedIndex == i);
                        if (ImGui::Selectable(registeredScenes[i].c_str(), isSelected)) {
                            selectedIndex = i;
                            selectedScene = registeredScenes[i];
                        }

                        // 選択中の項目にフォーカス
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::SameLine();

                // シーン変更ボタン
                bool isCurrent = scene_ && scene_->GetName() == selectedScene;
                if (isCurrent) {
                    ImGui::BeginDisabled();
                }

                if (ImGui::Button("Change Scene")) {
                    if (!selectedScene.empty() && !isCurrent) {
                        Change(selectedScene);
                    }
                }

                // 現在のシーンの場合はツールチップを表示
                if (isCurrent) {
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                        ImGui::BeginTooltip();
                        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "This scene is currently active");
                        ImGui::EndTooltip();
                    }
                    ImGui::EndDisabled();
                }
            } else {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "No scenes registered");
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Factory not initialized");
        }

        ImGui::End();
    });

    if (!scene_)return;
    context_.debug->RegisterCommand("Game", [this]() {
        scene_->Debug();
    });
}

const SceneSwitcher::Context& SceneSwitcher::GetContext() const {
    return context_;
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
