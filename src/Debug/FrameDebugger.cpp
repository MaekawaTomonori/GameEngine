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
    paused_ = false;
    stepRequested_ = false;
    frameCount_ = 0;
    debugUI_->RegisterMenuButton("FrameDebugger", false, "Debug");
#endif
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

            // >| ボタン：押した瞬間に1フレーム進み、長押しで徐々に加速する
            ImGui::Button(">|");

            if (ImGui::IsItemActive()) {
                // 長押し加速の定数
                constexpr int kRampFrames = 120;  // 最大速度に到達するまでのフレーム数

                // startInterval: maxSpeedInterval_ より遅い初期速度（最低30）
                const int startInterval = std::max(30, maxSpeedInterval_);
                // minInterval: maxSpeedInterval_ = 0 の場合でも1以上
                const int minInterval   = std::max(1, maxSpeedInterval_);

                if (holdFrames_ == 0) {
                    // 最初のフレーム: 即座に1フレーム進む
                    stepRequested_  = true;
                    lastStepFrame_  = 0;
                } else {
                    const int framesSinceStep = holdFrames_ - lastStepFrame_;

                    // holdFrames_ が startInterval を超えてから加速フェーズへ
                    const int rampFrames = holdFrames_ - startInterval;
                    int interval;
                    if (rampFrames <= 0) {
                        interval = startInterval;
                    } else if (rampFrames >= kRampFrames) {
                        interval = minInterval;
                    } else {
                        // startInterval → minInterval に線形補間
                        interval = startInterval
                            - (startInterval - minInterval) * rampFrames / kRampFrames;
                    }
                    interval = std::max(1, interval);

                    if (framesSinceStep >= interval) {
                        stepRequested_ = true;
                        lastStepFrame_ = holdFrames_;
                    }
                }
                ++holdFrames_;
            } else {
                // ボタンが離された: 状態リセット
                holdFrames_    = 0;
                lastStepFrame_ = -1;
            }

            // 最大速度スライダー (0 = 毎フレーム, 60 = 60フレームに1回)
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120.f);
            ImGui::SliderInt("Max interval", &maxSpeedInterval_, 0, 60);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("0-60frame");
            }
        }

        ImGui::End();
    });
#endif
}
