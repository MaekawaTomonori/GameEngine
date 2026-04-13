#include "Ui/UiManager.hpp"

#include <algorithm>
#include <filesystem>

#include "DebugUI.hpp"
#include "imgui.h"
#include "Log.hpp"

namespace Ui {

    void Manager::Setup(const GESTD::ReferencePtr<DebugUI>& _debugUi) {
        if (!_debugUi) return;

        debugUI_ = _debugUi;
        debugUI_->RegisterMenuButton("UI");
    }

    void Manager::Update() {
        for (const auto& entry : entries_) {
            if (!entry.canvas) continue;
            if (!entry.canvas->IsActive()) continue;

            entry.canvas->Update();
        }
    }

    void Manager::Draw() const {
        for (const auto& entry : entries_) {
            if (!entry.canvas) continue;
            if (!entry.canvas->IsActive()) continue;

            entry.canvas->Draw();
        }
    }

    void Manager::ScanJsonFiles() {
        namespace fs = std::filesystem;
        constexpr std::string_view dir = "Assets/Data/UI/";

        std::error_code ec;
        if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) return;

        for (const auto& fsEntry : fs::directory_iterator(dir, ec)) {
            if (ec) break;
            if (fsEntry.path().extension() != ".json") continue;

            const std::string name = fsEntry.path().stem().string();

            const bool exists = std::any_of(entries_.begin(), entries_.end(),
                [&name](const Entry& e) { return e.name == name; });
            if (!exists) {
                entries_.push_back({name, nullptr});
            }
        }
    }

    void Manager::Register(const std::string& _name, const GESTD::ReferencePtr<Canvas>& _canvas) {
        // 同名エントリがあれば canvas を更新（JSON スキャン後のロード時など）
        for (auto& entry : entries_) {
            if (entry.name == _name) {
                entry.canvas = _canvas;
                return;
            }
        }
        // 重複ポインタチェック
        for (const auto& entry : entries_) {
            if (static_cast<Canvas*>(entry.canvas) == static_cast<Canvas*>(_canvas)) return;
        }
        entries_.push_back({_name, _canvas});
    }

    GESTD::ReferencePtr<Canvas> Manager::GetCanvas(const std::string& _name) const {
        for (const auto& entry : entries_) {
            if (entry.name == _name) return entry.canvas;
        }
        return nullptr;
    }

    void Manager::Debug() {
        ScanJsonFiles();
        debugUI_->RegisterCommand("UI", [this]() {
            DrawListWindow();
            DrawDetailWindow();
        });
    }

    void Manager::LoadCanvas(int _index) {
        if (_index < 0 || _index >= static_cast<int>(entries_.size())) return;
        if (entries_[_index].IsLoaded()) return;

        auto canvas = std::make_unique<Canvas>();
        canvas->Setup(entries_[_index].name); // Setup 内の Register が entries_[_index].canvas を更新する
        owned_.push_back(std::move(canvas));
    }

    void Manager::UnloadCanvas(int _index) {
        if (!IsOwned(_index)) return;

        Canvas* target = static_cast<Canvas*>(entries_[_index].canvas);
        const auto it = std::find_if(owned_.begin(), owned_.end(),
            [target](const auto& p) { return p.get() == target; });
        if (it != owned_.end()) {
            owned_.erase(it); // デストラクタ → センチネル発火 → entries_[_index].canvas が自動的に無効化
        }
    }

    bool Manager::IsOwned(int _index) const {
        if (_index < 0 || _index >= static_cast<int>(entries_.size())) return false;
        Canvas* ptr = static_cast<Canvas*>(entries_[_index].canvas);
        if (!ptr) return false;
        return std::any_of(owned_.begin(), owned_.end(),
            [ptr](const auto& p) { return p.get() == ptr; });
    }

    void Manager::CreateCanvas(const std::string& _name) {
        if (_name.empty()) return;

        for (const auto& entry : entries_) {
            if (entry.name == _name) {
                Log::Send(Log::Level::WARNING, "UiManager: \"" + _name + "\" is already registered.");
                return;
            }
        }

        auto canvas = std::make_unique<Canvas>();
        canvas->Setup(_name); // Setup 内で Register が呼ばれる
        owned_.push_back(std::move(canvas));
    }

    void Manager::DrawListWindow() {
        ImGui::Begin("UI", &debugUI_->IsVisible("UI"));

        if (ImGui::Button("Scan")) ScanJsonFiles();
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.f);
        ImGui::InputText("##newname", newNameBuff_.data(), newNameBuff_.size());
        ImGui::SameLine();
        if (ImGui::Button("New")) {
            CreateCanvas(std::string(newNameBuff_.data()));
            newNameBuff_.fill(0);
        }

        ImGui::Separator();

        if (entries_.empty()) {
            ImGui::TextDisabled("No UI found. Press [Scan].");
            ImGui::End();
            return;
        }

        for (int i = 0; i < static_cast<int>(entries_.size()); ++i) {
            auto& entry = entries_[i];

            ImGui::PushID(i);

            const bool loaded   = entry.IsLoaded();
            const bool selected = (i == selectedIndex_);
            const bool active   = loaded && entry.canvas->IsActive();
            const bool owned    = IsOwned(i);

            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.20f, 0.45f, 0.75f, 0.80f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.30f, 0.55f, 0.85f, 0.80f));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.15f, 0.38f, 0.65f, 0.80f));
            }

            std::string label = loaded ? (active ? "[on]  " : "[--]  ") : "[ ]   ";
            label += entry.name + "###hdr";

            const bool headerOpen = ImGui::CollapsingHeader(label.c_str());
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Assets/Data/UI/%s.json", entry.name.c_str());
            }

            if (selected) ImGui::PopStyleColor(3);

            if (headerOpen) {
                if (loaded) ImGui::Text("Elements: %zu", entry.canvas->GetElementCount());
                else        ImGui::TextDisabled("Not loaded");

                if (ImGui::Button("Select")) selectedIndex_ = i;

                if (loaded) {
                    ImGui::SameLine();
                    if (ImGui::Button(active ? "Deactivate" : "Activate")) {
                        entry.canvas->SetActive(!active);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Save"))   entry.canvas->Save();
                    ImGui::SameLine();
                    if (ImGui::Button("Reload")) entry.canvas->Reload();

                    if (owned) {
                        ImGui::SameLine();
                        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.25f, 0.05f, 1.f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.35f, 0.1f,  1.f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.45f, 0.15f, 0.0f,  1.f));
                        if (ImGui::Button("Unload")) {
                            if (selectedIndex_ == i) selectedIndex_ = -1;
                            UnloadCanvas(i);
                        }
                        ImGui::PopStyleColor(3);
                    }
                }
            }

            ImGui::PopID();
        }

        ImGui::End();
    }

    void Manager::DrawDetailWindow() {
        if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(entries_.size())) return;

        // 未ロードなら遅延ロード
        if (!entries_[selectedIndex_].IsLoaded()) {
            LoadCanvas(selectedIndex_);
        }
        if (!entries_[selectedIndex_].IsLoaded()) return;

        bool open = true;
        entries_[selectedIndex_].canvas->Editor(&open);
        if (!open) {
            selectedIndex_ = -1;
        }
    }

} // namespace Ui
