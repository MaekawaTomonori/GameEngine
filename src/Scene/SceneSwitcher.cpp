#include "SceneSwitcher.hpp"

#include "imgui.h"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Director/CameraDirector.hpp"
#include "src/Debug/FrameDebugger.hpp"
#include "src/Light/LightManager.hpp"
#include "src/Scene/Transition/Transition.hpp"

#undef min
#undef max

SceneSwitcher::SceneSwitcher() {
    factory_ = std::make_unique<SceneFactory>();
}

void SceneSwitcher::Setup(const Context& _context) {
    context_ = _context;

    transition_ = std::make_unique<Transition>();
    transition_->Initialize();

#ifdef _DEBUG
    if (context_.debug) {
        context_.debug->RegisterMenuButton("SceneSwitcher");
        context_.debug->RegisterMenuButton("Transition");
    }

    Change("sample");
#endif
}

void SceneSwitcher::Update() {
    // Scene Change/ in Transition process
    // Transition in/out
    if (transition_->InProgress()){
        transition_->Update();
        return;
    }

    // Intra-scene transition: Out完了 → コールバック発火 → In開始
    if (midpointPending_) {
        midpointPending_ = false;
        if (midpointCallback_) {
            midpointCallback_();
            midpointCallback_ = nullptr;
        }
        transition_->Awake(intraInType_, ITransitionEffect::State::In);
        transition_->Update();
        return;
    }

    if (next_){
        midpointCallback_ = nullptr;
        if (scene_){
            scene_->Finalize();
            scene_.reset();
            scene_ = nullptr;
            Singleton<CameraDirector>::GetInstance()->Stop();
            Singleton<LightManager>::GetInstance()->ClearRef();
            if (context_.particle) {
                context_.particle->ClearActive();
            }
        }
        scene_ = std::move(next_);

        next_.reset();
        next_ = nullptr;
        scene_->Setup(this);
        scene_->Initialize();

        // Break on Load: シーン初期化直後に一時停止
        if (pendingBreak_ && context_.frame) {
            context_.frame->Pause();
#ifdef _DEBUG
            debugRunState_ = RunState::Paused;
#endif
        }
        pendingBreak_ = false;

        transition_->Awake(scene_->GetEntryTransition(), ITransitionEffect::State::In);
        transition_->Update();
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
        static int selectedIndex = -1;
        static std::string selectedScene;

        ImGui::Begin("SceneSwitcher", &context_.debug->IsVisible("SceneSwitcher"));

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
                // 初回のみ: 現在のシーンにselectedIndexを合わせる
                if (selectedIndex < 0) {
                    const std::string currentName = scene_ ? scene_->GetName() : std::string{};
                    selectedIndex = 0;
                    for (int i = 0; i < static_cast<int>(registeredScenes.size()); ++i) {
                        if (registeredScenes[i] == currentName) {
                            selectedIndex = i;
                            selectedScene = currentName;
                            break;
                        }
                    }
                }

                // コンボボックス用のプレビュー文字列
                const char* previewValue = selectedIndex < static_cast<int>(registeredScenes.size())
                    ? registeredScenes[selectedIndex].c_str()
                    : "Select Scene";

                if (ImGui::BeginCombo("##Scene List", previewValue)) {
                    for (int i = 0; i < static_cast<int>(registeredScenes.size()); ++i) {
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

    context_.debug->RegisterCommand("Transition", [this]() {
        ImGui::Begin("Transition", &context_.debug->IsVisible("Transition"));
        transition_->Debug();
        ImGui::Text("Callback Pending: %s", midpointPending_ ? "Yes" : "No");
        ImGui::End();
    });

    if (!scene_)return;
    context_.debug->RegisterCommand("Game", [this]() {
        scene_->Debug();
    });

    LocalDebugger();
}

void SceneSwitcher::SkipTransition() {
    if (transition_) {
        transition_->Skip();
    }
    midpointPending_  = false;
    midpointCallback_ = nullptr;
}

const SceneSwitcher::Context& SceneSwitcher::GetContext() const {
    return context_;
}

void SceneSwitcher::PlayTransition(Transition::Type _outType, Transition::Type _inType, std::function<void()> _onMidpoint) {
    if (transition_->InProgress()) return;
    intraOutType_ = _outType;
    intraInType_  = _inType;
    midpointCallback_ = std::move(_onMidpoint);
    midpointPending_ = true;
    transition_->Awake(_outType, ITransitionEffect::State::Out);
    transition_->Update(); 
}

void SceneSwitcher::PlayTransition(Transition::Type _type, std::function<void()> _onMidpoint) {
    PlayTransition(_type, _type, std::move(_onMidpoint));
}

void SceneSwitcher::Change(const std::string &_name) {
    if (!factory_) return;

    // Already Registered
    if (next_) return;

    next_ = factory_->Create(_name);
    next_->SetName(_name);

#ifdef _DEBUG
    debugRunState_ =  Utils::EqualsIgnoreCase(homeScene_, _name) ? RunState::Stopped : RunState::Running;
#endif

    // Already Destroyed
    if (!scene_) return;

    // Scene Change Transition
    transition_->Awake(scene_->GetExitTransition(), ITransitionEffect::State::Out);
}

void SceneSwitcher::LocalDebugger() {
#ifdef _DEBUG
    if (!context_.debug) return;

    context_.debug->RegisterCommand("FrameDebugger", [this]() {
        if (!ImGui::Begin("LocalDebugger", &context_.debug->IsVisible("FrameDebugger"))) {
            ImGui::End();
            return;
        }

        // homeScene を除外した起動候補リスト
        const auto allScenes = factory_ ? factory_->GetRegisteredScenes() : std::vector<std::string>{};
        std::vector<std::string> scenes;
        for (const auto& s : allScenes) {
            if (!Utils::EqualsIgnoreCase(s, homeScene_)) scenes.push_back(s);
        }

        static int  selectedIndex = -1;
        static bool breakOnLoad   = false;

        if (selectedIndex < 0 && !scenes.empty()) {
            const std::string cur = scene_ ? scene_->GetName() : std::string{};
            selectedIndex = 0;
            for (int i = 0; i < static_cast<int>(scenes.size()); ++i) {
                if (Utils::EqualsIgnoreCase(scenes[i], cur)) { selectedIndex = i; break; }
            }
        }
        selectedIndex = std::min(selectedIndex, static_cast<int>(scenes.size()) - 1);

        if (!scenes.empty()) {
            // コンボ: 停止中のみ有効
            if (debugRunState_ != RunState::Stopped) ImGui::BeginDisabled();
            ImGui::SetNextItemWidth(150.f);
            const char* preview = selectedIndex >= 0 ? scenes[selectedIndex].c_str() : "---";
            if (ImGui::BeginCombo("##Launch", preview)) {
                for (int i = 0; i < static_cast<int>(scenes.size()); ++i) {
                    if (ImGui::Selectable(scenes[i].c_str(), selectedIndex == i))
                        selectedIndex = i;
                    if (selectedIndex == i) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (debugRunState_ != RunState::Stopped) ImGui::EndDisabled();

            ImGui::SameLine();

            switch (debugRunState_) {
            case RunState::Stopped:
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.50f, 0.15f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.65f, 0.20f, 1.0f));
                if (selectedIndex >= 0 && ImGui::Button("> ##Launch")) {
                    SkipTransition();
                    if (context_.frame) context_.frame->Resume();
                    pendingBreak_ = breakOnLoad;
                    Change(scenes[selectedIndex]);
                }
                ImGui::PopStyleColor(2);
                if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Launch"); }

                ImGui::SameLine();

                ImGui::Checkbox("##breakOnLoad", &breakOnLoad);
                if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Break on Scene Start"); }

                break;

            case RunState::Running:
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.08f, 0.08f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.12f, 0.12f, 1.0f));
                if (ImGui::Button("||##Pause")) {
                    debugRunState_ = RunState::Paused;
                    if (context_.frame) context_.frame->Pause();
                }
                ImGui::PopStyleColor(2);
                if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Pause"); }
                break;

            case RunState::Paused:
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.50f, 0.12f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.65f, 0.18f, 1.0f));
                if (ImGui::Button("> ##Resume")) {
                    debugRunState_ = RunState::Running;
                    if (context_.frame) context_.frame->Resume();
                }
                ImGui::PopStyleColor(2);
                ImGui::SameLine();
                if (context_.frame) context_.frame->RenderStepButton();
                break;
            }
        }

        const bool isHome = !homeScene_.empty() && scene_ && scene_->GetName() == homeScene_;

        ImGui::SameLine();

        if (isHome) ImGui::BeginDisabled();

        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.50f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.68f, 0.16f, 0.16f, 1.0f));
        if (ImGui::Button("##Stop", ImVec2(22.f, 0.f))) {
            debugRunState_ = RunState::Stopped;
            SkipTransition();
            if (context_.frame) context_.frame->Resume();
            Change(homeScene_);
        }
        ImGui::PopStyleColor(2);

        {
            constexpr float kPad = 5.f;
            const ImVec2 bMin = ImGui::GetItemRectMin();
            const ImVec2 bMax = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRectFilled(
                { bMin.x + kPad, bMin.y + kPad },
                { bMax.x - kPad, bMax.y - kPad },
                IM_COL32(220, 220, 220, 220)
            );
        }

        if (isHome) {
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip("Already at home scene");
            }
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (ImGui::Button("Re")) {
            debugRunState_ = RunState::Running;
            SkipTransition();
            if (context_.frame) context_.frame->Resume();
            pendingBreak_ = breakOnLoad;
            if (scene_) Change(scene_->GetName());
        }
        ImGui::End();
    });
#endif
}
