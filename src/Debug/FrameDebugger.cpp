#include "FrameDebugger.hpp"

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
