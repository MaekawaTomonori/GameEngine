#ifdef _DEBUG

#include "PerformanceProfiler.hpp"

#include "DebugUI.hpp"
#include "imgui.h"

#include <algorithm>

void PerformanceProfiler::Initialize(DebugUI* _debug) {
    debugUI_ = _debug;
    if (debugUI_) {
        debugUI_->RegisterMenuButton("Profiler", false, "Debug");
    }
}

void PerformanceProfiler::Begin(const char* _name) {
    active_[_name] = std::chrono::high_resolution_clock::now();
}

void PerformanceProfiler::End(const char* _name) {
    const auto it = active_.find(_name);
    if (it == active_.end()) return;

    const double ms =
        std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - it->second
        ).count();
    active_.erase(it);

    auto& s = samples_[_name];
    s.smoothMs = (s.smoothMs == 0.0) ? ms : s.smoothMs * 0.95 + ms * 0.05;
    s.lastMs   = ms;

    if (std::find(order_.begin(), order_.end(), _name) == order_.end()) {
        order_.push_back(_name);
    }
}

void PerformanceProfiler::Debug() {
    if (!debugUI_) return;

    debugUI_->RegisterCommand("Profiler", [this]() {
        ImGui::Begin("Profiler", &debugUI_->IsVisible("Profiler"));

        if (order_.empty()) {
            ImGui::TextDisabled("No sections recorded.");
        } else if (ImGui::BeginTable("##ProfTable", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Section",   ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Last (ms)", ImGuiTableColumnFlags_WidthFixed, 72.f);
            ImGui::TableSetupColumn("Avg  (ms)", ImGuiTableColumnFlags_WidthFixed, 72.f);
            ImGui::TableHeadersRow();

            for (const auto& name : order_) {
                const auto& s = samples_.at(name);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(name.c_str());
                ImGui::TableSetColumnIndex(1); ImGui::Text("%.3f", s.lastMs);
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.3f", s.smoothMs);
            }
            ImGui::EndTable();
        }

        ImGui::End();
    });
}

#endif // _DEBUG
