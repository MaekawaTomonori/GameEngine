#include "FrameDebugger.hpp"

#ifdef _DEBUG
#include "imgui.h"
#include "DebugUI.hpp"

#undef max
#undef min
#endif

void FrameDebugger::Initialize([[maybe_unused]] DebugUI* _debugUi) {
#ifdef _DEBUG
    if (!_debugUi) return;
    debugUI_ = _debugUi;
    debugUI_->RegisterMenuButton("FrameDebugger", false, "Debug");
#endif
}

void FrameDebugger::Pause() {
#ifdef _DEBUG
    paused_ = true;
#endif
}

void FrameDebugger::Resume() {
#ifdef _DEBUG
    paused_ = false;
#endif
}

bool FrameDebugger::ShouldUpdate() {
#ifdef _DEBUG
    const bool run = !paused_ || stepRequested_;
    stepRequested_ = false;
    return run;
#else
    return true;
#endif
}

void FrameDebugger::RenderStepButton() {
#ifdef _DEBUG
    // [>|(薄青)] → 1フレームステップ（長押しで加速）
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.28f, 0.58f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.38f, 0.72f, 1.0f));
    ImGui::Button(">|");
    ImGui::PopStyleColor(2);

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
