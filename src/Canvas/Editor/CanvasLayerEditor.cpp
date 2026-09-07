#include "CanvasLayerEditor.hpp"

#include <cstring>
#include <d3d12.h>

#include "DebugUI.hpp"
#include "Factory/PostEffectFactory.hpp"
#include "imgui.h"
#include "src/Canvas/Canvas.hpp"
#include "src/PostProcess/Chain/PostEffectChain.hpp"
#include "src/PostProcess/Editor/PostEffectChainEditorUI.hpp"
#include "src/PostProcess/Executor/PostProcessExecutor.hpp"
#include "src/PostProcess/IPostEffect.hpp"

namespace {
    /** @brief Overlay行の選択状態を他Canvasと同じマップで管理するための予約キー */
    constexpr const char* OVERLAY_KEY = "Overlay";
} // namespace

void CanvasLayerEditor::Initialize(const GESTD::ReferencePtr<DebugUI>& _debug, const GESTD::ReferencePtr<PostProcessExecutor>& _executor) {
    debug_ = _debug;
    executor_ = _executor;
}

void CanvasLayerEditor::ShowEditor() {
    if (!debug_) return;

    // 現在編集中のCanvasの詳細ウィンドウは、メインエディタの開閉に関わらず毎フレーム登録する
    // （ただし編集対象は常に1つだけで、DebugUIの通常ウィンドウ一覧には登録しない）
    RenderEditingCanvasWindow();

    if (!showEditor_) return;

    debug_->RegisterCommand("Canvas Editor", [this]() {
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Canvas Editor", &showEditor_);

        RenderCanvasList();
        ImGui::Separator();
        RenderAddCanvasSection();

        ImGui::End();
    });
}

void CanvasLayerEditor::OpenEditor() {
    showEditor_ = true;
}

void CanvasLayerEditor::CloseEditor() {
    showEditor_ = false;
}

void CanvasLayerEditor::RenderCanvasList() {
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Canvases");
    ImGui::Separator();

    if (ImGui::BeginTable("CanvasTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        ImGui::TableHeadersRow();

        // Overlay: 通常のCanvasとは異なる特殊な存在なので、常に先頭・削除不可の行として固定表示する
        {
            ImGui::PushID(OVERLAY_KEY);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", OVERLAY_KEY);

            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled("-");

            ImGui::TableSetColumnIndex(2);
            if (ImGui::SmallButton("Edit")) {
                editingOverlay_ = true;
                editingCanvasName_.clear();
            }

            ImGui::PopID();
        }

        std::string toRemove;

        for (const auto& [zOrder, canvas] : executor_->GetCanvases()) {
            ImGui::PushID(canvas);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", canvas->GetName().c_str());

            ImGui::TableSetColumnIndex(1);
            const bool enabled = canvas->IsEnabled();
            if (enabled) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
                if (ImGui::SmallButton("ON")) canvas->SetEnabled(false);
                ImGui::PopStyleColor(2);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                if (ImGui::SmallButton("OFF")) canvas->SetEnabled(true);
                ImGui::PopStyleColor(2);
            }

            ImGui::TableSetColumnIndex(2);
            if (ImGui::SmallButton("Edit")) {
                // Canvasの詳細編集は1つずつ。別のCanvas/Overlayを選ぶと前の編集ウィンドウは自動的に閉じる
                editingCanvasName_ = canvas->GetName();
                editingOverlay_ = false;
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            if (ImGui::SmallButton("X")) {
                toRemove = canvas->GetName();
            }
            ImGui::PopStyleColor();

            ImGui::PopID();
        }

        ImGui::EndTable();

        if (!toRemove.empty()) {
            if (editingCanvasName_ == toRemove) {
                editingCanvasName_.clear();
            }
            executor_->RemoveCanvas(toRemove);
        }
    }
}

void CanvasLayerEditor::RenderAddCanvasSection() {
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Add Canvas");
    ImGui::Separator();

    ImGui::SetNextItemWidth(200);
    ImGui::InputTextWithHint("##NewCanvasName", "New canvas name", newCanvasNameBuf_, sizeof(newCanvasNameBuf_));
    ImGui::SameLine();

    if (ImGui::Button("Add")) {
        if (strlen(newCanvasNameBuf_) > 0) {
            executor_->AddCanvas(newCanvasNameBuf_);
            newCanvasNameBuf_[0] = '\0';
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Add (Top)")) {
        if (strlen(newCanvasNameBuf_) > 0) {
            executor_->AddCanvasTop(newCanvasNameBuf_);
            newCanvasNameBuf_[0] = '\0';
        }
    }
}

void CanvasLayerEditor::RenderEditingCanvasWindow() {
    if (!editingOverlay_ && editingCanvasName_.empty()) return;

    // 固定のウィンドウ名でコマンド登録する（DebugUIの通常ウィンドウ一覧＝RegisterMenuButtonには登録しない）。
    // 編集対象はOverlayかCanvasのどちらか1つだけに限定し、他のウィンドウ管理システムには一切依存しない。
    debug_->RegisterCommand("Canvas Detail", [this]() {
        if (editingOverlay_) {
            PostEffectChain* overlayChain = executor_->GetOverlayChain();
            if (!overlayChain) {
                editingOverlay_ = false;
                return;
            }

            bool open = true;
            ImGui::SetNextWindowSize(ImVec2(420, 520), ImGuiCond_FirstUseEver);
            ImGui::Begin("Canvas Detail - Overlay###CanvasDetailWindow", &open);

            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Overlay (applies to the final composited screen)");
            ImGui::Separator();
            RenderPostEffectChainEditor(overlayChain, executor_->GetFactory(), selectedEffectByCanvas_[OVERLAY_KEY], selectedEffectTypeByCanvas_[OVERLAY_KEY]);

            ImGui::End();

            if (!open) {
                editingOverlay_ = false;
            }
            return;
        }

        Canvas* canvas = executor_->GetCanvas(editingCanvasName_);
        if (!canvas) {
            // 編集中に削除された等でCanvasが見つからない場合は編集状態を解除する
            editingCanvasName_.clear();
            return;
        }

        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(420, 520), ImGuiCond_FirstUseEver);
        // "###CanvasDetailWindow" でID部分を固定し、表示名だけCanvas切替に追従させる（位置・サイズを保持するため）
        ImGui::Begin(("Canvas Detail - " + canvas->GetName() + "###CanvasDetailWindow").c_str(), &open);

        RenderCanvasDetail(canvas);

        ImGui::End();

        if (!open) {
            editingCanvasName_.clear();
        }
    });
}

void CanvasLayerEditor::RenderCanvasDetail(Canvas* _canvas) {
    bool canvasEnabled = _canvas->IsEnabled();
    if (ImGui::Checkbox("Enable", &canvasEnabled)) {
        _canvas->SetEnabled(canvasEnabled);
    }

    const uint64_t previewTextureId = ResolvePreviewTextureId(_canvas);
    if (previewTextureId != 0) {
        const float aspect = _canvas->GetChain()->GetAspectRatio();
        const float availWidth = ImGui::GetContentRegionAvail().x;
        const ImVec2 imageSize(availWidth, aspect > 0.0f ? availWidth / aspect : availWidth);
        ImGui::Image(static_cast<ImTextureID>(previewTextureId), imageSize);
    }

    ImGui::Spacing();
    ImGui::Separator();

    RenderPostEffectChainEditor(_canvas->GetChain(), executor_->GetFactory(),
        selectedEffectByCanvas_[_canvas->GetName()], selectedEffectTypeByCanvas_[_canvas->GetName()]);
}

uint64_t CanvasLayerEditor::ResolvePreviewTextureId(Canvas* _canvas) {
    if (!debug_) return 0;

    ID3D12Resource* resource = _canvas->GetChain()->GetPreviewResource();
    if (!resource) return 0;

    PreviewCacheEntry& entry = previewCache_[_canvas->GetName()];
    if (entry.resource != resource) {
        entry.resource = resource;
        entry.textureId = debug_->RegisterTexture(resource, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    }
    return entry.textureId;
}
