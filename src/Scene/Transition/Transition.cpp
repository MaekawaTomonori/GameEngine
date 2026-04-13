#include "Transition.hpp"
#include "Fade.hpp"

#include "imgui.h"
#include "Log.hpp"
#include "MagicEnum/magic_enum.hpp"

void Transition::Initialize() {
    type_ = Type::None;
}

void Transition::Update() {
    if (!effect_) return;
    effect_->Update();
}

void Transition::Draw() {
    if (effect_) {
        effect_->Draw();
    }
}

void Transition::Awake(const Type _type, const ITransitionEffect::State _state) {
    if (_type == Type::None) {
        // In が None の場合: エフェクトを即時リセットしてαを0にする
        if (_state == ITransitionEffect::State::In && effect_) {
            effect_->Stop();
        }
        return;
    }
    Awake(_type, _state, defaultDuration_);
}

void Transition::Awake(const Type _type, const ITransitionEffect::State _state, const float _duration) {
    if (_type == Type::None){
        effect_.reset();
        return;
    }
    if (type_ != _type){
        type_ = _type;
        CreateEffect(_type);
        effect_->Initialize();
    }
    if (effect_) {
        effect_->Start(_state, _duration);
        Log::Send(Log::Level::INFO,
            "[Transition] " +
            std::string(magic_enum::enum_name(_type)) + " " +
            std::string(magic_enum::enum_name(_state)) +
            " (duration=" + std::to_string(_duration) + "s)");
    }
}

bool Transition::InProgress() {
    if (awakePending_) {
        awakePending_ = false;
        Awake(pendingType_, pendingState_);
    }
    
    if (!effect_) {
        return false;
    }

    return !effect_->IsFinished();
}

void Transition::Skip() {
    if (effect_) {
        effect_->Stop();
    }
    awakePending_ = false;
}

void Transition::SetDefaultDuration(const float _duration) {
    defaultDuration_ = _duration;
}

ITransitionEffect::State Transition::GetCurrentState() const {
    if (!effect_) return ITransitionEffect::State::None;
    return effect_->GetCurrentState();
}

float Transition::GetProgress() const {
    if (!effect_) return 0.0f;
    return effect_->GetProgress();
}

void Transition::Debug() {
    auto typeName  = magic_enum::enum_name(type_);
    auto stateName = magic_enum::enum_name(GetCurrentState());

    ImGui::SeparatorText("Transition");
    ImGui::Text("Type  : %.*s", static_cast<int>(typeName.size()),  typeName.data());
    ImGui::Text("State : %.*s", static_cast<int>(stateName.size()), stateName.data());
    ImGui::Text("Active: %s", InProgress() ? "Yes" : "No");

    if (effect_ && InProgress()) {
        float progress = GetProgress();
        ImGui::ProgressBar(progress, ImVec2(-1.f, 0.f));
        ImGui::Text("Alpha : %.3f", progress);
        ImGui::SameLine();
        if (ImGui::Button("Skip")) { Skip(); }
    }

    ImGui::SeparatorText("Debug Trigger");

    static int debugTypeIdx  = 1; // 0=None はスキップ、初期値 Fade
    static int debugStateIdx = 1; // 0=None はスキップ、初期値 In

    constexpr auto typeValues  = magic_enum::enum_values<Type>();
    constexpr auto stateValues = magic_enum::enum_values<ITransitionEffect::State>();

    // Type コンボ（None を除く index 1 以降）
    const char* typePreview = magic_enum::enum_name(typeValues[debugTypeIdx]).data();
    ImGui::SetNextItemWidth(80.f);
    if (ImGui::BeginCombo("##type", typePreview)) {
        for (int i = 1; i < static_cast<int>(typeValues.size()); ++i) {
            const bool selected = (debugTypeIdx == i);
            if (ImGui::Selectable(magic_enum::enum_name(typeValues[i]).data(), selected)) {
                debugTypeIdx = i;
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    // State コンボ（None を除く index 1 以降）
    const char* statePreview = magic_enum::enum_name(stateValues[debugStateIdx]).data();
    ImGui::SetNextItemWidth(70.f);
    if (ImGui::BeginCombo("##state", statePreview)) {
        for (int i = 1; i < static_cast<int>(stateValues.size()); ++i) {
            const bool selected = (debugStateIdx == i);
            if (ImGui::Selectable(magic_enum::enum_name(stateValues[i]).data(), selected)) {
                debugStateIdx = i;
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    const bool inProgress = InProgress();
    if (inProgress) ImGui::BeginDisabled();
    if (ImGui::Button("Play##dbg")) {
        pendingType_  = typeValues[debugTypeIdx];
        pendingState_ = stateValues[debugStateIdx];
        awakePending_ = true;
    }
    if (inProgress) ImGui::EndDisabled();
}

void Transition::CreateEffect(const Type _type) {
    std::unique_ptr<ITransitionEffect> _effect = nullptr;
    switch (_type) {
        case Type::Fade:
            _effect = std::make_unique<Fade>();
            break;
        case Type::Slide:
            break;
        case Type::None:
            break;
        default: ;
    }
    if (effect_)effect_.reset();
    effect_ = std::move(_effect);
}
