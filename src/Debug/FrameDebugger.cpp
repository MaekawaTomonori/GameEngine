#include "FrameDebugger.hpp"

#ifdef _DEBUG
#include "imgui.h"
#endif
#include "DebugUI.hpp"

void FrameDebugger::Initialize(DebugUI* _debugUi) {
    if (!_debugUi) return;
    debugUI_ = _debugUi;
    paused_ = false;
    stepRequested_ = false;
    frameCount_ = 0;
    debugUI_->RegisterMenuButton("FrameDebugger");
}

bool FrameDebugger::ShouldUpdate() {
#ifdef _DEBUG
    bool frag = !paused_ || stepRequested_;

    if (stepRequested_) {
        stepRequested_ = false;
    }

    return frag;
#else
    return true;
#endif
}

void FrameDebugger::Debug() {
#ifdef _DEBUG
    if (!debugUI_) return;

    debugUI_->RegisterCommand("FrameDebugger", [this]() {
        ImGui::Begin("FrameStepDebugger", &debugUI_->IsVisible("FrameDebugger"));

        if (!paused_) {
            if (ImGui::Button("||")) {
                paused_ = true;
            }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            if (ImGui::Button("||")) {
                paused_ = false;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::Button(">|")) {
                stepRequested_ = true;
            }
        }

        ImGui::End();
    });
#endif
}
