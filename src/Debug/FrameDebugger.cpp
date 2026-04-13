#include "FrameDebugger.hpp"

#include "imgui_internal.h"

#ifdef _DEBUG
#include "imgui.h"
#include "DebugUI.hpp"

#undef max
#undef min
#endif

void FrameDebugger::Initialize([[maybe_unused]]const GESTD::ReferencePtr<DebugUI>& _debugUi) {
#ifdef _DEBUG
    if (!_debugUi) return;
    debugUI_ = _debugUi;
#endif
}

void FrameDebugger::Play() {
#ifdef _DEBUG
    state_ = State::Running;
#endif
}

void FrameDebugger::Pause() {
#ifdef _DEBUG
    if (state_ == State::Running) state_ = State::Paused;
#endif
}

void FrameDebugger::Stop() {
#ifdef _DEBUG
    state_          = State::Stopped;
    stopFramesLeft_ = 1;   // 1フレームだけ通してシーンリセットを伝搬させる
    if (onStop_) onStop_();
#endif
}

void FrameDebugger::SetStopCallback([[maybe_unused]] const std::function<void()>& _cb) {
#ifdef _DEBUG
    onStop_ = std::move(_cb);
#endif
}

//void FrameDebugger::RenderToolbar() {
//#ifdef _DEBUG
//    // 停止中: 赤ハイライト  それ以外: 暗色
//    ImGui::PushStyleColor(ImGuiCol_Button,
//        state_ == State::Stopped ? ImVec4(0.35f, 0.08f, 0.08f, 1.0f)
//                                 : ImVec4(0.10f, 0.10f, 0.10f, 1.0f));
//    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
//    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.28f, 0.06f, 0.06f, 1.0f));
//    ImGui::PushStyleColor(ImGuiCol_Text,
//        state_ == State::Stopped ? ImVec4(0.95f, 0.35f, 0.35f, 1.0f)
//                                 : ImVec4(0.38f, 0.38f, 0.38f, 1.0f));
//    if (ImGui::Button(" [] ") && state_ != State::Stopped) Stop();
//    ImGui::PopStyleColor(4);
//
//    ImGui::SameLine(0.f, 4.f);
//    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
//    ImGui::SameLine(0.f, 4.f);
//
//    // 実行中: 緑ハイライト  それ以外: 暗色
//    ImGui::PushStyleColor(ImGuiCol_Button,
//        state_ == State::Running ? ImVec4(0.07f, 0.28f, 0.09f, 1.0f)
//                                 : ImVec4(0.10f, 0.10f, 0.10f, 1.0f));
//    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.40f, 0.13f, 1.0f));
//    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.05f, 0.20f, 0.07f, 1.0f));
//    ImGui::PushStyleColor(ImGuiCol_Text,
//        state_ == State::Running ? ImVec4(0.50f, 0.95f, 0.55f, 1.0f)
//                                 : ImVec4(0.38f, 0.38f, 0.38f, 1.0f));
//    if (ImGui::Button("  >  ") && state_ != State::Running) Play();
//    ImGui::PopStyleColor(4);
//
//    ImGui::SameLine(0.f, 2.f);
//
//    // 一時停止中: アンバーハイライト  それ以外: 暗色
//    ImGui::PushStyleColor(ImGuiCol_Button,
//        state_ == State::Paused ? ImVec4(0.30f, 0.20f, 0.02f, 1.0f)
//                                : ImVec4(0.10f, 0.10f, 0.10f, 1.0f));
//    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.28f, 0.04f, 1.0f));
//    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.22f, 0.14f, 0.02f, 1.0f));
//    ImGui::PushStyleColor(ImGuiCol_Text,
//        state_ == State::Paused ? ImVec4(0.95f, 0.76f, 0.20f, 1.0f)
//                                : ImVec4(0.38f, 0.38f, 0.38f, 1.0f));
//    if (ImGui::Button(" || ") && state_ == State::Running) Pause();
//    ImGui::PopStyleColor(4);
//
//    ImGui::SameLine(0.f, 2.f);
//
//    // Paused 以外はグレーアウト
//    if (state_ != State::Paused) ImGui::BeginDisabled();
//    RenderStepButton();
//    if (state_ != State::Paused) ImGui::EndDisabled();
//#endif
//}

bool FrameDebugger::ShouldUpdate() {
#ifdef _DEBUG
    ++frameCount_;
    switch (state_) {
    case State::Running:
        return true;
    case State::Stopped:
        if (stopFramesLeft_ > 0) { --stopFramesLeft_; return true; }
        return false;
    case State::Paused: 
        const bool run = stepRequested_;
        stepRequested_ = false;
        return run;
    }
    return false;
#else
    return true;
#endif
}

void FrameDebugger::RenderStepButton() {
#ifdef _DEBUG
    // >| ステップ（長押し加速）: 薄いブルーホバーでツールバーと区別
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f, 0.10f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.30f, 0.52f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.12f, 0.20f, 0.38f, 1.0f));
    ImGui::Button(">|");
    ImGui::PopStyleColor(3);

    if (ImGui::IsItemActive()) {
        constexpr int kRampFrames = 120;
        const int startInterval = std::max(30, maxSpeedInterval_);
        const int minInterval   = std::max(1,  maxSpeedInterval_);

        if (holdFrames_ == 0) {
            stepRequested_ = true;
            lastStepFrame_ = 0;
        } else {
            const int framesSinceStep = holdFrames_ - lastStepFrame_;
            const int rampFrames      = holdFrames_ - startInterval;
            int interval;
            if      (rampFrames <= 0)           interval = startInterval;
            else if (rampFrames >= kRampFrames) interval = minInterval;
            else    interval = startInterval - (startInterval - minInterval) * rampFrames / kRampFrames;

            if (framesSinceStep >= std::max(1, interval)) {
                stepRequested_ = true;
                lastStepFrame_ = holdFrames_;
            }
        }
        ++holdFrames_;
    } else {
        holdFrames_    = 0;
        lastStepFrame_ = -1;
    }
#endif
}
