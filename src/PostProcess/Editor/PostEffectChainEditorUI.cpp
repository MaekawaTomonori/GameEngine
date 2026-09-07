#include "PostEffectChainEditorUI.hpp"

#include <vector>

#include "Factory/PostEffectFactory.hpp"
#include "imgui.h"
#include "src/PostProcess/Chain/PostEffectChain.hpp"
#include "src/PostProcess/IPostEffect.hpp"

void RenderPostEffectChainEditor(PostEffectChain* _chain, const GESTD::ReferencePtr<PostEffectFactory>& _factory, std::string& _selectedEffect, int& _selectedNewEffectType) {
    if (!_factory) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "PostEffectFactory is not set.");
    } else {
        auto availableTypes = _factory->GetRegisteredTypes();
        if (availableTypes.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "No effect types available!");
        } else {
            std::vector<const char*> typesCStr;
            for (const auto& type : availableTypes) {
                typesCStr.push_back(type.c_str());
            }

            if (_selectedNewEffectType >= static_cast<int>(availableTypes.size())) _selectedNewEffectType = 0;

            ImGui::SetNextItemWidth(200);
            ImGui::Combo("##NewEffectType", &_selectedNewEffectType, typesCStr.data(), static_cast<int>(typesCStr.size()));
            ImGui::SameLine();
            if (ImGui::Button("Add Effect")) {
                _chain->Create(availableTypes[_selectedNewEffectType]);
            }
        }
    }

    auto& effects = _chain->GetEffects();

    if (ImGui::BeginTable("EffectChainTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg, ImVec2(0, 150))) {
        ImGui::TableSetupColumn("Effect", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableHeadersRow();

        for (auto& effect : effects) {
            ImGui::PushID(effect.type.c_str());
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            const bool isSelected = (_selectedEffect == effect.type);
            if (ImGui::Selectable(effect.type.c_str(), isSelected)) {
                _selectedEffect = isSelected ? "" : effect.type;
            }

            ImGui::TableSetColumnIndex(1);
            if (effect.enabled) {
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Yes");
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "No");
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    const bool hasSelection = !_selectedEffect.empty();
    if (!hasSelection) ImGui::BeginDisabled();

    if (ImGui::Button("Up")) _chain->MoveEffectUp(_selectedEffect);
    ImGui::SameLine();
    if (ImGui::Button("Down")) _chain->MoveEffectDown(_selectedEffect);

    if (hasSelection) {
        for (auto& effect : effects) {
            if (effect.type != _selectedEffect) continue;
            ImGui::SameLine();
            ImGui::Checkbox("Enabled", &effect.enabled);
            break;
        }
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
    if (ImGui::Button("Remove")) {
        _chain->RemoveEffect(_selectedEffect);
        _selectedEffect.clear();
    }
    ImGui::PopStyleColor();

    if (!hasSelection) {
        ImGui::EndDisabled();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select an effect above to edit it.");
    } else {
        ImGui::Separator();
        ImGui::BeginChild("EffectChainParamsChild", ImVec2(0, 180), true);
        for (auto& effect : effects) {
            if (effect.type != _selectedEffect) continue;
            effect.effect->Debug();
            break;
        }
        ImGui::EndChild();
    }

    ImGui::Spacing();
    if (ImGui::Button("Save Config")) {
        _chain->SavePermanentConfig();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Config")) {
        _chain->LoadPermanentConfig();
    }
}
