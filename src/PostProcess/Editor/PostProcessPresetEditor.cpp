#include "PostProcessPresetEditor.hpp"

#include <algorithm>
#include <cctype>
#include <format>
#include <fstream>
#include <filesystem>
#include "DebugUI.hpp"
#include "Log.hpp"
#include "imgui.h"
#include "src/PostProcess/Executor/PostProcessExecutor.hpp"

void PostProcessPresetEditor::Initialize(DebugUI* _debug, PostProcessExecutor* _executor) {
    debug_ = _debug;
    executor_ = _executor;
}

void PostProcessPresetEditor::ShowEditor() {
    if (!debug_) return;

    debug_->RegisterCommand("PostEffect Preset Editor", [this](){
        if (!showEditor_) return;

        ImGui::SetNextWindowSize(ImVec2(900, 750), ImGuiCond_FirstUseEver);
        ImGui::Begin("PostEffect Preset Editor", &showEditor_);

        // Main Tabs
        if (ImGui::BeginTabBar("PresetEditorTabs", ImGuiTabBarFlags_None)) {

            // Tab 1: Preset Configuration
            if (ImGui::BeginTabItem("Preset Configuration")) {
                RenderAvailablePresetsSection();
                ImGui::Separator();

                RenderCreateNewPresetSection();
                ImGui::Separator();

                RenderPresetConfigurationSection();
                ImGui::Separator();

                RenderQuickSaveSection();

                ImGui::EndTabItem();
            }

            // Tab 2: Keyframe Points
            if (ImGui::BeginTabItem("Keyframe Points")) {
                RenderKeyframePointsTab();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    });
}

void PostProcessPresetEditor::RenderAvailablePresetsSection() {
    if (!ImGui::CollapsingHeader("Available Presets", ImGuiTreeNodeFlags_DefaultOpen)) return;

    // Search and Sort Controls
    ImGui::InputTextWithHint("##Search", "Search presets...", searchBuffer_, sizeof(searchBuffer_));
    ImGui::SameLine();

    const char* sortModes[] = { "Name", "Duration", "Members" };
    ImGui::SetNextItemWidth(120);
    ImGui::Combo("Sort", &sortMode_, sortModes, IM_ARRAYSIZE(sortModes));

    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        // Dynamic reload on next frame
    }

    // Preview indicator
    if (isPreviewingPreset_) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[Previewing: %s]", previewingPresetName_.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton("Stop Preview")) {
            StopPreview();
        }
    }

    ImGui::Separator();

    auto presets = GetFilteredAndSortedPresets();
    ImGui::Text("Showing: %zu preset(s)", presets.size());

    if (presets.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No presets found. Create one to get started!");
        return;
    }

    // Presets List
    ImGui::BeginChild("PresetsList", ImVec2(0, 250), true);

    static std::string selectedPresetForDuplicate = "";
    static bool showDuplicateDialog = false;

    for (const auto& presetName : presets) {
        ImGui::PushID(presetName.c_str());

        bool isCurrentlyEditing = (isEditingPreset_ && editingPreset_.name == presetName);
        bool isPreviewing = (isPreviewingPreset_ && previewingPresetName_ == presetName);

        // Get preset info
        auto info = GetPresetInfo(presetName);

        // Status indicator
        if (isCurrentlyEditing) {
            ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "[Edit]");
        } else if (isPreviewing) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[Play]");
        } else {
            ImGui::Text("      ");
        }
        ImGui::SameLine();

        // Preset name
        ImGui::Text("%s", presetName.c_str());

        // Preset info (duration, member count, mode)
        ImGui::SameLine(250);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                          "(%.1fs | %d fx | %s)",
                          info.duration,
                          info.memberCount,
                          info.mode == "maintain_state" ? "M" : "D");

        // Action buttons
        ImGui::SameLine(380);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        if (ImGui::SmallButton("Apply")) {
            ApplyPreset(presetName);
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();

        if (ImGui::SmallButton("Preview")) {
            PreviewPreset(presetName);
        }
        ImGui::SameLine();

        if (ImGui::SmallButton("Edit")) {
            if (!isEditingPreset_) {
                LoadPresetForEditing(presetName);
            }
        }
        ImGui::SameLine();

        if (ImGui::SmallButton("Duplicate")) {
            selectedPresetForDuplicate = presetName;
            showDuplicateDialog = true;
        }
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        if (ImGui::SmallButton("Delete")) {
            if (!isCurrentlyEditing) {
                DeletePreset(presetName);
            }
        }
        ImGui::PopStyleColor();

        ImGui::PopID();
    }
    ImGui::EndChild();

    // Duplicate Dialog
    if (showDuplicateDialog) {
        ImGui::OpenPopup("DuplicatePresetDialog");
        showDuplicateDialog = false;
    }

    if (ImGui::BeginPopupModal("DuplicatePresetDialog", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Duplicate Preset: %s", selectedPresetForDuplicate.c_str());
        ImGui::Separator();

        static char newNameBuf[256] = "";
        static std::string errorMsg = "";

        ImGui::InputText("New Name", newNameBuf, sizeof(newNameBuf));

        if (!errorMsg.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", errorMsg.c_str());
        }

        ImGui::Spacing();

        if (ImGui::Button("Duplicate", ImVec2(120, 0))) {
            std::string newName = newNameBuf;
            errorMsg.clear();

            if (ValidatePresetName(newName, errorMsg)) {
                DuplicatePreset(selectedPresetForDuplicate, newName);
                newNameBuf[0] = '\0';
                errorMsg.clear();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            newNameBuf[0] = '\0';
            errorMsg.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void PostProcessPresetEditor::RenderCreateNewPresetSection() {
    if (!ImGui::CollapsingHeader("Create New Preset")) return;

    static char nameBuffer[256] = "";
    static std::string createErrorMsg = "";

    ImGui::InputText("Preset Name", nameBuffer, sizeof(nameBuffer));

    if (!createErrorMsg.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", createErrorMsg.c_str());
    }

    ImGui::SameLine();

    if (ImGui::Button("Create")) {
        if (!isEditingPreset_) {
            std::string newName = nameBuffer;
            createErrorMsg.clear();

            if (ValidatePresetName(newName, createErrorMsg)) {
                CreateNewPreset(newName);
                nameBuffer[0] = '\0';
                createErrorMsg.clear();
            }
        }
    }
}

void PostProcessPresetEditor::RenderPresetConfigurationSection() {
    if (!isEditingPreset_) return;
    if (!ImGui::CollapsingHeader("Preset Configuration", ImGuiTreeNodeFlags_DefaultOpen)) return;

    // Header with Save/Cancel buttons
    ImGui::Text("Editing: %s", editingPreset_.name.c_str());

    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        SaveEditingPreset();
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
    if (ImGui::Button("Apply Current")) {
        // 現在の編集状態を一時保存してから適用
        SaveEditingPreset();
        ApplyPreset(editingPreset_.name);
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        StopEditingPreset();
    }

    ImGui::Separator();

    // Basic Settings
    RenderBasicSettings();
    ImGui::Spacing();

    // Members List
    RenderMembersList();
    ImGui::Spacing();

    // Ignore List
    RenderIgnoreList();
}

void PostProcessPresetEditor::RenderBasicSettings() {
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Basic Settings");
    ImGui::Separator();

    // Duration
    ImGui::DragFloat("Duration (seconds)", &editingPreset_.duration, 0.1f, 0.0f, 10.0f, "%.2f");

    // Mode selection
    const char* modeItems[] = { "maintain_state", "disable_unlisted" };
    int currentMode = (editingPreset_.mode == "disable_unlisted") ? 1 : 0;
    if (ImGui::Combo("Mode", &currentMode, modeItems, IM_ARRAYSIZE(modeItems))) {
        editingPreset_.mode = modeItems[currentMode];
    }

    // Help text
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("maintain_state: Keep unlisted effects in current state");
        ImGui::Text("disable_unlisted: Disable all effects not in members list");
        ImGui::EndTooltip();
    }
}

void PostProcessPresetEditor::RenderMembersList() {
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Effect Members");
    ImGui::Separator();

    ImGui::Text("Members (%zu)", editingPreset_.members.size());
    ImGui::SameLine();
    if (ImGui::Button("Add Member")) {
        showAddMemberDialog_ = true;
    }

    // Add Member Dialog
    if (showAddMemberDialog_) {
        ImGui::OpenPopup("AddMemberDialog");
        showAddMemberDialog_ = false;
    }

    RenderAddMemberDialog();

    // Members List
    ImGui::BeginChild("MembersListChild", ImVec2(0, 200), true);
    if (editingPreset_.members.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No members. Click 'Add Member' to start.");
    } else {
        int indexToRemove = -1;
        int indexToMoveUp = -1;
        int indexToMoveDown = -1;

        for (int i = 0; i < static_cast<int>(editingPreset_.members.size()); ++i) {
            ImGui::PushID(i);
            auto& member = editingPreset_.members[i];

            bool isSelected = (selectedMemberIndex_ == i);

            // Reorder buttons
            if (i > 0) {
                if (ImGui::SmallButton("↑")) {
                    indexToMoveUp = i;
                }
            } else {
                ImGui::TextDisabled(" ");
            }
            ImGui::SameLine();

            if (i < static_cast<int>(editingPreset_.members.size()) - 1) {
                if (ImGui::SmallButton("↓")) {
                    indexToMoveDown = i;
                }
            } else {
                ImGui::TextDisabled(" ");
            }
            ImGui::SameLine();

            // Selectable member item
            ImGui::PushStyleColor(ImGuiCol_Header, isSelected ? ImVec4(0.3f, 0.5f, 0.8f, 0.8f) : ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
            if (ImGui::Selectable(member.name.c_str(), isSelected, 0, ImVec2(180, 0))) {
                selectedMemberIndex_ = i;
            }
            ImGui::PopStyleColor();

            ImGui::SameLine(250);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(%s)", member.type.c_str());

            ImGui::SameLine();
            if (member.autoCreate) {
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[Auto]");
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "[Manual]");
            }

            ImGui::SameLine(450);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            if (ImGui::SmallButton("Remove")) {
                indexToRemove = i;
            }
            ImGui::PopStyleColor();

            ImGui::PopID();
        }

        // Deferred operations
        if (indexToMoveUp >= 0) {
            MoveMemberUp(indexToMoveUp);
        }
        if (indexToMoveDown >= 0) {
            MoveMemberDown(indexToMoveDown);
        }
        if (indexToRemove >= 0) {
            RemoveMember(indexToRemove);
        }
    }
    ImGui::EndChild();

    // Selected Member Details
    if (selectedMemberIndex_ >= 0 && selectedMemberIndex_ < static_cast<int>(editingPreset_.members.size())) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Member Details");
        ImGui::Separator();

        auto& selectedMember = editingPreset_.members[selectedMemberIndex_];

        // Name editing
        char nameBuf[128];
        strncpy_s(nameBuf, selectedMember.name.c_str(), sizeof(nameBuf) - 1);
        nameBuf[sizeof(nameBuf) - 1] = '\0';
        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
            selectedMember.name = std::string(nameBuf);
        }

        // Type display (read-only)
        ImGui::Text("Type: %s", selectedMember.type.c_str());

        // Auto-create flag
        ImGui::Checkbox("Auto Create", &selectedMember.autoCreate);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("If true, effect will be created automatically if it doesn't exist");
            ImGui::EndTooltip();
        }
    }
}

void PostProcessPresetEditor::RenderIgnoreList() {
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Ignore List");
    ImGui::Separator();

    ImGui::Text("Ignored Effects (%zu)", editingPreset_.ignoreList.size());
    ImGui::SameLine();
    if (ImGui::Button("Add Ignored Effect")) {
        ImGui::OpenPopup("AddIgnoreDialog");
    }

    // Add Ignore Dialog
    if (ImGui::BeginPopup("AddIgnoreDialog")) {
        ImGui::Text("Add Effect to Ignore List");
        ImGui::Separator();

        static char ignoreNameBuf[128] = "";
        ImGui::InputText("Effect Name", ignoreNameBuf, sizeof(ignoreNameBuf));

        if (ImGui::Button("Add", ImVec2(120, 0))) {
            if (strlen(ignoreNameBuf) > 0) {
                editingPreset_.ignoreList.push_back(std::string(ignoreNameBuf));
                ignoreNameBuf[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Ignore List Display
    ImGui::BeginChild("IgnoreListChild", ImVec2(0, 100), true);
    if (editingPreset_.ignoreList.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No ignored effects.");
    } else {
        int ignoreIndexToRemove = -1;

        for (int i = 0; i < static_cast<int>(editingPreset_.ignoreList.size()); ++i) {
            ImGui::PushID(i);

            ImGui::Text("%s", editingPreset_.ignoreList[i].c_str());
            ImGui::SameLine(300);

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            if (ImGui::SmallButton("Remove")) {
                ignoreIndexToRemove = i;
            }
            ImGui::PopStyleColor();

            ImGui::PopID();
        }

        // Deferred deletion
        if (ignoreIndexToRemove >= 0) {
            editingPreset_.ignoreList.erase(editingPreset_.ignoreList.begin() + ignoreIndexToRemove);
        }
    }
    ImGui::EndChild();
}

void PostProcessPresetEditor::RenderAddMemberDialog() {
    if (ImGui::BeginPopup("AddMemberDialog")) {
        ImGui::Text("Add New Member");
        ImGui::Separator();

        static int selectedType = 0;
        auto availableTypes = GetAvailableEffectTypes();

        if (!availableTypes.empty()) {
            // Type selection
            std::vector<const char*> typesCStr;
            for (const auto& type : availableTypes) {
                typesCStr.push_back(type.c_str());
            }
            ImGui::Combo("Type", &selectedType, typesCStr.data(), static_cast<int>(typesCStr.size()));

            // Name input
            static char memberNameBuf[128] = "";
            ImGui::InputText("Name", memberNameBuf, sizeof(memberNameBuf));

            // Auto-create checkbox
            static bool autoCreate = true;
            ImGui::Checkbox("Auto Create", &autoCreate);

            ImGui::Spacing();

            if (ImGui::Button("Add", ImVec2(120, 0))) {
                if (strlen(memberNameBuf) > 0 && selectedType >= 0 && selectedType < static_cast<int>(availableTypes.size())) {
                    AddMember(availableTypes[selectedType], std::string(memberNameBuf), autoCreate);
                    memberNameBuf[0] = '\0';
                    autoCreate = true;
                    selectedType = 0;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "No effect types available!");
        }

        ImGui::EndPopup();
    }
}

void PostProcessPresetEditor::RenderQuickSaveSection() {
    if (!ImGui::CollapsingHeader("Quick Save Current State")) return;

    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Save current scene's post-effect state");
    ImGui::Text("Steps:");
    ImGui::BulletText("1. Set up post-effects in your scene");
    ImGui::BulletText("2. Adjust parameters in PostEffect window");
    ImGui::BulletText("3. Enter name below and save");

    ImGui::Spacing();

    static char saveNameBuf[256] = "";
    ImGui::InputText("Save As", saveNameBuf, sizeof(saveNameBuf));
    ImGui::SameLine();
    if (ImGui::Button("Save Scene State", ImVec2(150, 0))) {
        if (strlen(saveNameBuf) > 0) {
            executor_->SavePreset(std::string(saveNameBuf));
            Log::Send(Log::Level::INFO, std::format("Saved scene state as '{}'", saveNameBuf));
            saveNameBuf[0] = '\0';
        }
    }
}

void PostProcessPresetEditor::OpenEditor(const std::string& _presetName) {
    showEditor_ = true;
    if (!_presetName.empty()) {
        LoadPresetForEditing(_presetName);
    }
}

void PostProcessPresetEditor::CloseEditor() {
    showEditor_ = false;
    isEditingPreset_ = false;
}

void PostProcessPresetEditor::LoadPresetForEditing(const std::string& _presetName) {
    std::string path = "./Assets/Data/PostEffect/presets.json";

    if (!std::filesystem::exists(path)) {
        Log::Send(Log::Level::ERR, "presets.json not found");
        return;
    }

    std::ifstream file(path);
    nlohmann::json presetsJson;
    file >> presetsJson;
    file.close();

    if (!presetsJson.contains(_presetName)) {
        Log::Send(Log::Level::ERR, std::format("Preset '{}' not found", _presetName));
        return;
    }

    auto presetJson = presetsJson[_presetName];

    // Load preset data
    editingPreset_.name = _presetName;
    editingPreset_.duration = presetJson.value("duration", 2.0f);
    editingPreset_.mode = presetJson.value("mode", "maintain_state");
    editingPreset_.members.clear();
    editingPreset_.ignoreList.clear();

    // Load members
    if (presetJson.contains("members")) {
        for (const auto& memberJson : presetJson["members"]) {
            PresetMember member;
            member.type = memberJson.value("type", "");
            member.name = memberJson.value("name", "");
            member.autoCreate = memberJson.value("autoCreate", true);
            editingPreset_.members.push_back(member);
        }
    }

    // Load ignore list
    if (presetJson.contains("ignoreList")) {
        for (const auto& ignoreName : presetJson["ignoreList"]) {
            editingPreset_.ignoreList.push_back(ignoreName);
        }
    }

    isEditingPreset_ = true;
    selectedMemberIndex_ = -1;

    Log::Send(Log::Level::INFO, std::format("Loaded preset '{}' for editing", _presetName));
}

void PostProcessPresetEditor::SaveEditingPreset() {
    if (!isEditingPreset_) return;

    std::string path = "./Assets/Data/PostEffect/presets.json";

    // Load existing presets
    nlohmann::json allPresets;
    if (std::filesystem::exists(path)) {
        std::ifstream inFile(path);
        inFile >> allPresets;
        inFile.close();
    }

    // Build preset JSON
    nlohmann::json presetJson;
    presetJson["duration"] = editingPreset_.duration;
    presetJson["mode"] = editingPreset_.mode;

    // Serialize members
    nlohmann::json membersArray = nlohmann::json::array();
    for (const auto& member : editingPreset_.members) {
        nlohmann::json memberJson;
        memberJson["type"] = member.type;
        memberJson["name"] = member.name;
        memberJson["autoCreate"] = member.autoCreate;
        membersArray.push_back(memberJson);
    }
    presetJson["members"] = membersArray;

    // Serialize ignore list
    nlohmann::json ignoreArray = nlohmann::json::array();
    for (const auto& ignoreName : editingPreset_.ignoreList) {
        ignoreArray.push_back(ignoreName);
    }
    presetJson["ignoreList"] = ignoreArray;

    // Update presets
    allPresets[editingPreset_.name] = presetJson;

    // Save to file
    std::filesystem::create_directories("./Assets/Data/PostEffect");
    std::ofstream outFile(path);
    outFile << allPresets.dump(4);
    outFile.close();

    Log::Send(Log::Level::INFO, std::format("Saved preset '{}'", editingPreset_.name));
}

void PostProcessPresetEditor::CreateNewPreset(const std::string& _name) {
    editingPreset_.name = _name.empty() ? "NewPreset" : _name;
    editingPreset_.duration = 2.0f;
    editingPreset_.mode = "maintain_state";
    editingPreset_.members.clear();
    editingPreset_.ignoreList.clear();
    selectedMemberIndex_ = -1;
    isEditingPreset_ = true;

    Log::Send(Log::Level::INFO, std::format("Created new preset '{}'", editingPreset_.name));
}

void PostProcessPresetEditor::StopEditingPreset() {
    isEditingPreset_ = false;
    editingPreset_ = PresetData();
    selectedMemberIndex_ = -1;

    Log::Send(Log::Level::INFO, "Stopped editing preset");
}

void PostProcessPresetEditor::DeletePreset(const std::string& _presetName) {
    std::string path = "./Assets/Data/PostEffect/presets.json";

    if (!std::filesystem::exists(path)) {
        Log::Send(Log::Level::ERR, "presets.json not found");
        return;
    }

    // Load existing presets
    std::ifstream inFile(path);
    nlohmann::json allPresets;
    inFile >> allPresets;
    inFile.close();

    // Remove the preset
    if (allPresets.contains(_presetName)) {
        allPresets.erase(_presetName);

        // Save back
        std::ofstream outFile(path);
        outFile << allPresets.dump(4);
        outFile.close();

        Log::Send(Log::Level::INFO, std::format("Deleted preset '{}'", _presetName));
    } else {
        Log::Send(Log::Level::WARNING, std::format("Preset '{}' not found for deletion", _presetName));
    }
}

void PostProcessPresetEditor::AddMember(const std::string& _type, const std::string& _name, bool _autoCreate) {
    PresetMember member;
    member.type = _type;
    member.name = _name;
    member.autoCreate = _autoCreate;
    editingPreset_.members.push_back(member);

    Log::Send(Log::Level::INFO, std::format("Added member '{}' ({})", _name, _type));
}

void PostProcessPresetEditor::RemoveMember(int _index) {
    if (_index < 0 || _index >= static_cast<int>(editingPreset_.members.size())) {
        return;
    }

    editingPreset_.members.erase(editingPreset_.members.begin() + _index);

    // Adjust selectedMemberIndex_
    if (selectedMemberIndex_ == _index) {
        selectedMemberIndex_ = -1;
    } else if (selectedMemberIndex_ > _index) {
        selectedMemberIndex_--;
    }

    Log::Send(Log::Level::INFO, "Removed member from preset");
}

std::vector<std::string> PostProcessPresetEditor::GetAvailablePresets() const {
    std::vector<std::string> presets;

    std::string path = "./Assets/Data/PostEffect/presets.json";
    if (!std::filesystem::exists(path)) {
        return presets;
    }

    std::ifstream file(path);
    nlohmann::json presetsJson;
    file >> presetsJson;
    file.close();

    for (auto it = presetsJson.begin(); it != presetsJson.end(); ++it) {
        presets.push_back(it.key());
    }

    return presets;
}

std::vector<std::string> PostProcessPresetEditor::GetAvailableEffectTypes() const {
    // TODO: This should query PostProcessExecutor's factory for available types
    // For now, return hardcoded list
    return { "Vignette", "Grayscale", "BoxBlur" };
}

void PostProcessPresetEditor::MoveMemberUp(int _index) {
    if (_index <= 0 || _index >= static_cast<int>(editingPreset_.members.size())) {
        return;
    }

    std::swap(editingPreset_.members[_index], editingPreset_.members[_index - 1]);

    // Update selected index
    if (selectedMemberIndex_ == _index) {
        selectedMemberIndex_ = _index - 1;
    } else if (selectedMemberIndex_ == _index - 1) {
        selectedMemberIndex_ = _index;
    }

    Log::Send(Log::Level::INFO, "Moved member up");
}

void PostProcessPresetEditor::MoveMemberDown(int _index) {
    if (_index < 0 || _index >= static_cast<int>(editingPreset_.members.size()) - 1) {
        return;
    }

    std::swap(editingPreset_.members[_index], editingPreset_.members[_index + 1]);

    // Update selected index
    if (selectedMemberIndex_ == _index) {
        selectedMemberIndex_ = _index + 1;
    } else if (selectedMemberIndex_ == _index + 1) {
        selectedMemberIndex_ = _index;
    }

    Log::Send(Log::Level::INFO, "Moved member down");
}

void PostProcessPresetEditor::DuplicatePreset(const std::string& _sourceName, const std::string& _newName) {
    std::string path = "./Assets/Data/PostEffect/presets.json";

    if (!std::filesystem::exists(path)) {
        Log::Send(Log::Level::ERR, "presets.json not found");
        return;
    }

    std::ifstream inFile(path);
    nlohmann::json allPresets;
    inFile >> allPresets;
    inFile.close();

    if (!allPresets.contains(_sourceName)) {
        Log::Send(Log::Level::ERR, std::format("Source preset '{}' not found", _sourceName));
        return;
    }

    if (allPresets.contains(_newName)) {
        Log::Send(Log::Level::ERR, std::format("Preset '{}' already exists", _newName));
        return;
    }

    // Copy preset data
    allPresets[_newName] = allPresets[_sourceName];

    // Save
    std::ofstream outFile(path);
    outFile << allPresets.dump(4);
    outFile.close();

    Log::Send(Log::Level::INFO, std::format("Duplicated preset '{}' to '{}'", _sourceName, _newName));
}

void PostProcessPresetEditor::PreviewPreset(const std::string& _presetName) {
    if (!executor_) return;

    executor_->ApplyPreset(_presetName);
    previewingPresetName_ = _presetName;
    isPreviewingPreset_ = true;

    Log::Send(Log::Level::INFO, std::format("Previewing preset '{}'", _presetName));
}

void PostProcessPresetEditor::StopPreview() {
    isPreviewingPreset_ = false;
    previewingPresetName_.clear();

    Log::Send(Log::Level::INFO, "Stopped preview");
}

bool PostProcessPresetEditor::ValidatePresetName(const std::string& _name, std::string& _errorMsg) const {
    if (_name.empty()) {
        _errorMsg = "Preset name cannot be empty";
        return false;
    }

    if (_name.find_first_of("/\\:*?\"<>|") != std::string::npos) {
        _errorMsg = "Preset name contains invalid characters";
        return false;
    }

    auto existingPresets = GetAvailablePresets();
    if (std::find(existingPresets.begin(), existingPresets.end(), _name) != existingPresets.end()) {
        _errorMsg = "Preset name already exists";
        return false;
    }

    return true;
}

PostProcessPresetEditor::PresetInfo PostProcessPresetEditor::GetPresetInfo(const std::string& _presetName) const {
    PresetInfo info;
    info.name = _presetName;
    info.memberCount = 0;
    info.duration = 0.0f;
    info.mode = "unknown";

    std::string path = "./Assets/Data/PostEffect/presets.json";
    if (!std::filesystem::exists(path)) {
        return info;
    }

    std::ifstream file(path);
    nlohmann::json presetsJson;
    file >> presetsJson;
    file.close();

    if (!presetsJson.contains(_presetName)) {
        return info;
    }

    auto presetJson = presetsJson[_presetName];
    info.duration = presetJson.value("duration", 0.0f);
    info.mode = presetJson.value("mode", "unknown");

    if (presetJson.contains("members")) {
        info.memberCount = static_cast<int>(presetJson["members"].size());
    }

    return info;
}

std::vector<std::string> PostProcessPresetEditor::GetFilteredAndSortedPresets() const {
    auto presets = GetAvailablePresets();

    // Filter by search text
    if (strlen(searchBuffer_) > 0) {
        std::string searchLower = searchBuffer_;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        presets.erase(
            std::remove_if(presets.begin(), presets.end(), [&](const std::string& name) {
                std::string nameLower = name;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return nameLower.find(searchLower) == std::string::npos;
            }),
            presets.end()
        );
    }

    // Sort by selected mode
    if (sortMode_ == 1) {
        // Sort by duration
        std::sort(presets.begin(), presets.end(), [this](const std::string& a, const std::string& b) {
            auto infoA = GetPresetInfo(a);
            auto infoB = GetPresetInfo(b);
            return infoA.duration < infoB.duration;
        });
    } else if (sortMode_ == 2) {
        // Sort by member count
        std::sort(presets.begin(), presets.end(), [this](const std::string& a, const std::string& b) {
            auto infoA = GetPresetInfo(a);
            auto infoB = GetPresetInfo(b);
            return infoA.memberCount < infoB.memberCount;
        });
    } else {
        // Sort by name (default)
        std::sort(presets.begin(), presets.end());
    }

    return presets;
}

// ===== Keyframe Points Tab Implementation =====

void PostProcessPresetEditor::RenderKeyframePointsTab() {
    if (!isEditingPreset_) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No preset selected for editing.");
        ImGui::Text("Please select a preset in the 'Preset Configuration' tab first.");
        return;
    }

    if (editingPreset_.members.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No members in preset.");
        ImGui::Text("Add members in the 'Preset Configuration' tab first.");
        return;
    }

    ImGui::Text("Editing Preset: %s", editingPreset_.name.c_str());
    ImGui::Separator();

    // Member selection
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Select Effect Member");
    ImGui::BeginChild("KeyframeMemberSelect", ImVec2(0, 150), true);

    for (int i = 0; i < static_cast<int>(editingPreset_.members.size()); ++i) {
        ImGui::PushID(i);
        auto& member = editingPreset_.members[i];

        bool isSelected = (selectedMemberIndex_ == i);

        ImGui::PushStyleColor(ImGuiCol_Header, isSelected ? ImVec4(0.3f, 0.5f, 0.8f, 0.8f) : ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
        if (ImGui::Selectable(member.name.c_str(), isSelected)) {
            selectedMemberIndex_ = i;
            LoadMemberKeyframes();
        }
        ImGui::PopStyleColor();

        ImGui::SameLine(250);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(%s)", member.type.c_str());

        ImGui::PopID();
    }

    ImGui::EndChild();

    // Keyframe editing for selected member
    if (selectedMemberIndex_ >= 0 && selectedMemberIndex_ < static_cast<int>(editingPreset_.members.size())) {
        ImGui::Separator();
        RenderPointsList();
        ImGui::Separator();
        RenderPointParameters();
    }
}

void PostProcessPresetEditor::RenderPointsList() {
    auto& selectedMember = editingPreset_.members[selectedMemberIndex_];

    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Keyframe Points for: %s", selectedMember.name.c_str());

    ImGui::Text("Points (%zu)", editingKeyframeOrder_.size());
    ImGui::SameLine();
    if (ImGui::Button("Add Point")) {
        ImGui::OpenPopup("AddPointDialog");
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload from JSON")) {
        LoadMemberKeyframes();
    }

    // Add Point Dialog
    if (ImGui::BeginPopupModal("AddPointDialog", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Add New Keyframe Point");
        ImGui::Separator();

        static char pointNameBuf[128] = "";
        ImGui::InputText("Point Name", pointNameBuf, sizeof(pointNameBuf));

        ImGui::Spacing();

        if (ImGui::Button("Add", ImVec2(120, 0))) {
            if (strlen(pointNameBuf) > 0) {
                AddKeyframePoint(std::string(pointNameBuf));
                pointNameBuf[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            pointNameBuf[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Points List
    ImGui::BeginChild("PointsList", ImVec2(0, 200), true);
    if (editingKeyframeOrder_.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No keyframe points. Click 'Add Point' to create one.");
    } else {
        int indexToRemove = -1;
        int indexToMoveUp = -1;
        int indexToMoveDown = -1;

        for (int i = 0; i < static_cast<int>(editingKeyframeOrder_.size()); ++i) {
            ImGui::PushID(i);
            const std::string& pointName = editingKeyframeOrder_[i];

            bool isSelected = (selectedPointIndex_ == i);

            // Reorder buttons
            if (i > 0) {
                if (ImGui::SmallButton("Up")) {
                    indexToMoveUp = i;
                }
            } else {
                ImGui::TextDisabled(" ");
            }
            ImGui::SameLine();

            if (i < static_cast<int>(editingKeyframeOrder_.size()) - 1) {
                if (ImGui::SmallButton("Dn")) {
                    indexToMoveDown = i;
                }
            } else {
                ImGui::TextDisabled(" ");
            }
            ImGui::SameLine();

            // Selectable point
            ImGui::PushStyleColor(ImGuiCol_Header, isSelected ? ImVec4(0.3f, 0.5f, 0.8f, 0.8f) : ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
            if (ImGui::Selectable(pointName.c_str(), isSelected, 0, ImVec2(200, 0))) {
                selectedPointIndex_ = i;
                // 自動プレビュー：選択時に即座にパラメータをロード
                PreviewCurrentPoint();
            }
            ImGui::PopStyleColor();

            ImGui::SameLine(300);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "[Order: %d]", i + 1);

            ImGui::SameLine(420);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            if (ImGui::SmallButton("Remove")) {
                indexToRemove = i;
            }
            ImGui::PopStyleColor();

            ImGui::PopID();
        }

        // Deferred operations
        if (indexToMoveUp >= 0) {
            MovePointUp(indexToMoveUp);
        }
        if (indexToMoveDown >= 0) {
            MovePointDown(indexToMoveDown);
        }
        if (indexToRemove >= 0) {
            RemoveKeyframePoint(indexToRemove);
        }
    }
    ImGui::EndChild();
}

void PostProcessPresetEditor::LoadMemberKeyframes() {
    if (!executor_) return;
    if (selectedMemberIndex_ < 0 || selectedMemberIndex_ >= static_cast<int>(editingPreset_.members.size())){
        return;
    }

    auto& member = editingPreset_.members[selectedMemberIndex_];

    // JSONファイルパス
    std::string keyframePath = std::format("./Assets/Data/PostEffect/{}/{}.json", member.type, editingPreset_.name);

    // ファイルが存在しない場合は空で初期化
    if (!std::filesystem::exists(keyframePath)){
        editingKeyframes_ = nlohmann::json::object();
        editingKeyframeOrder_.clear();
        selectedPointIndex_ = -1;
        keyframesDirty_ = false;
        Log::Send(Log::Level::INFO, std::format("No keyframe file found for '{}', starting empty", member.name));
        return;
    }

    // JSONファイルから一括読み込み
    std::ifstream file(keyframePath);
    if (!file.is_open()){
        Log::Send(Log::Level::ERR, std::format("Failed to open keyframe file: {}", keyframePath));
        return;
    }

    nlohmann::json fullJson;
    file >> fullJson;
    file.close();

    // editingKeyframes_にキーフレームデータをコピー（"keyframes"配列を除く）
    editingKeyframes_ = nlohmann::json::object();
    for (auto it = fullJson.begin(); it != fullJson.end(); ++it){
        if (it.key() != "keyframes" && it.value().is_object()){
            editingKeyframes_[it.key()] = it.value();
        }
    }

    // キーフレーム順序を読み込み
    editingKeyframeOrder_.clear();
    if (fullJson.contains("keyframes") && fullJson["keyframes"].is_array()){
        editingKeyframeOrder_ = fullJson["keyframes"].get<std::vector<std::string>>();
    }

    // 最初のポイントを選択
    if (!editingKeyframeOrder_.empty()){
        selectedPointIndex_ = 0;
        PreviewCurrentPoint();
    } else{
        selectedPointIndex_ = -1;
    }

    keyframesDirty_ = false;
    Log::Send(Log::Level::INFO, std::format("Loaded {} keyframe points for '{}'",
              editingKeyframes_.size(), member.name));
}

void PostProcessPresetEditor::SaveMemberKeyframes() {
    if (!executor_) return;
    if (selectedMemberIndex_ < 0 || selectedMemberIndex_ >= static_cast<int>(editingPreset_.members.size())){
        return;
    }

    auto& member = editingPreset_.members[selectedMemberIndex_];

    // ディレクトリ作成
    std::string dirPath = std::format("./Assets/Data/PostEffect/{}", member.type);
    std::filesystem::create_directories(dirPath);

    // JSONファイルパス
    std::string keyframePath = std::format("{}/{}.json", dirPath, editingPreset_.name);

    // 保存用JSON構築
    nlohmann::json saveJson = editingKeyframes_;  // 全キーフレームデータをコピー
    saveJson["keyframes"] = editingKeyframeOrder_;  // 順序配列を追加

    // ファイルに書き込み
    std::ofstream file(keyframePath);
    if (!file.is_open()){
        Log::Send(Log::Level::ERR, std::format("Failed to save keyframe file: {}", keyframePath));
        return;
    }

    file << saveJson.dump(4);  // インデント付き
    file.close();

    keyframesDirty_ = false;
    Log::Send(Log::Level::INFO, std::format("Saved {} keyframe points for '{}' to {}",
              editingKeyframes_.size(), member.name, keyframePath));
}

void PostProcessPresetEditor::PreviewCurrentPoint() {
    if (!executor_) return;
    if (selectedMemberIndex_ < 0 || selectedMemberIndex_ >= static_cast<int>(editingPreset_.members.size())){
        return;
    }
    if (selectedPointIndex_ < 0 || selectedPointIndex_ >= static_cast<int>(editingKeyframeOrder_.size())){
        return;
    }

    auto& member = editingPreset_.members[selectedMemberIndex_];
    const std::string& pointName = editingKeyframeOrder_[selectedPointIndex_];

    // エフェクトインスタンスを取得
    IPostEffect* effect = executor_->FindOrCreate(member.type, member.name, true);
    if (!effect){
        Log::Send(Log::Level::ERR, std::format("Failed to find/create effect '{}'", member.name));
        return;
    }

    // エフェクトにプリセット全体を読み込ませる
    effect->LoadPreset(editingPreset_.name);

    // JSONから読み込んだkeyframeOrder内でpointNameのインデックスを探す
    int actualIndex = selectedPointIndex_;  // 基本的には同じはず
    int totalKeyframes = static_cast<int>(editingKeyframeOrder_.size());

    // t値を計算（0.0~1.0）
    float t = 0.0f;
    if (totalKeyframes > 1){
        t = static_cast<float>(actualIndex) / static_cast<float>(totalKeyframes - 1);
    }

    // UpdateAnimation()で特定のキーフレームを適用
    effect->UpdateAnimation(t);

    Log::Send(Log::Level::DBG, std::format("Preview point '{}' (t={:.2f}) for '{}'",
              pointName, t, member.name));
}

void PostProcessPresetEditor::RenderPointParameters() {
    if (selectedPointIndex_ < 0 || selectedPointIndex_ >= static_cast<int>(editingKeyframeOrder_.size())){
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No point selected.");
        return;
    }

    const std::string& pointName = editingKeyframeOrder_[selectedPointIndex_];

    // キーフレームデータが存在しない場合
    if (!editingKeyframes_.contains(pointName)){
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Point data not found!");
        return;
    }

    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Editing Point: %s", pointName.c_str());
    ImGui::Separator();

    // ポイント名の編集
    char pointNameBuf[128];
    strncpy_s(pointNameBuf, pointName.c_str(), sizeof(pointNameBuf) - 1);
    pointNameBuf[sizeof(pointNameBuf) - 1] = '\0';

    if (ImGui::InputText("Point Name", pointNameBuf, sizeof(pointNameBuf))){
        std::string newName = pointNameBuf;
        if (newName != pointName && !newName.empty()){
            // 名前を変更
            editingKeyframes_[newName] = editingKeyframes_[pointName];
            editingKeyframes_.erase(pointName);
            editingKeyframeOrder_[selectedPointIndex_] = newName;
            keyframesDirty_ = true;
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Parameters");
    ImGui::Text("Edit parameters using the effect's UI below:");

    ImGui::BeginChild("PointParameterEdit", ImVec2(0, 250), true);

    // エフェクトインスタンスを取得して、そのDebug()を呼び出す
    if (!executor_ || selectedMemberIndex_ < 0 || selectedMemberIndex_ >= static_cast<int>(editingPreset_.members.size())) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Effect instance not available.");
    } else {
        auto& member = editingPreset_.members[selectedMemberIndex_];
        IPostEffect* effect = executor_->FindOrCreate(member.type, member.name, true);

        if (effect) {
            // エフェクトのDebug()を呼び出してパラメータを編集
            // これにより、全てのパラメータが編集可能になる
            effect->Debug();
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Failed to create effect instance.");
        }
    }

    ImGui::EndChild();

    ImGui::Spacing();

    // 保存ボタン - エフェクトの現在の状態をこのポイントに保存
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
    if (ImGui::Button("Save Current Parameters to Point", ImVec2(-1, 0))) {
        // エフェクトの現在のパラメータを取得してJSONに保存
        if (executor_ && selectedMemberIndex_ >= 0 && selectedMemberIndex_ < static_cast<int>(editingPreset_.members.size())) {
            auto& member = editingPreset_.members[selectedMemberIndex_];
            IPostEffect* effect = executor_->FindOrCreate(member.type, member.name, true);

            if (effect) {
                const std::string& currentPointName = editingKeyframeOrder_[selectedPointIndex_];

                // 一時的なプリセット名で現在の状態を保存
                std::string tempPresetName = "_editor_temp_" + currentPointName;
                effect->SavePreset(tempPresetName);

                // 保存された一時ファイルを読み込む
                std::string tempPath = std::format("./Assets/Data/PostEffect/{}/{}.json", member.type, tempPresetName);
                if (std::filesystem::exists(tempPath)) {
                    std::ifstream tempFile(tempPath);
                    nlohmann::json tempJson;
                    tempFile >> tempJson;
                    tempFile.close();

                    // 一時ファイルから現在のポイントのデータを抽出
                    // SaveParameters()が返すのは全キーフレーム構造なので、
                    // 任意のキーフレームデータを取り出す（実際のmaterial_の値）
                    bool foundData = false;
                    for (auto it = tempJson.begin(); it != tempJson.end(); ++it) {
                        if (it.key() != "keyframes" && it.value().is_object()) {
                            // これが実際のパラメータデータ
                            editingKeyframes_[currentPointName] = it.value();
                            foundData = true;
                            break;
                        }
                    }

                    // 一時ファイルを削除
                    std::filesystem::remove(tempPath);

                    if (foundData) {
                        // JSONファイルに保存
                        SaveMemberKeyframes();
                        Log::Send(Log::Level::INFO, std::format("Saved current parameters to point '{}'", currentPointName));
                    } else {
                        Log::Send(Log::Level::WARNING, "Could not extract parameter data from effect");
                    }
                } else {
                    Log::Send(Log::Level::ERR, std::format("Temp file not found: {}", tempPath));
                }
            }
        }
    }
    ImGui::PopStyleColor();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Click to save current effect state to this keyframe point");
}
void PostProcessPresetEditor::AddKeyframePoint(const std::string& pointName) {
    if (pointName.empty()) return;
    
    // 既に存在するか確認
    if (editingKeyframes_.contains(pointName)) {
        Log::Send(Log::Level::WARNING, std::format("Keyframe point '{}' already exists", pointName));
        return;
    }
    
    // 新しいポイントをデフォルト値で作成（Vignetteの例）
    nlohmann::json defaultParams;
    defaultParams["intensity"] = 16.0f;
    defaultParams["scale"] = 0.8f;
    defaultParams["color"] = {1.0f, 1.0f, 1.0f};
    
    editingKeyframes_[pointName] = defaultParams;
    editingKeyframeOrder_.push_back(pointName);
    
    keyframesDirty_ = true;
    
    Log::Send(Log::Level::INFO, std::format("Added keyframe point '{}'", pointName));
}

void PostProcessPresetEditor::RemoveKeyframePoint(int pointIndex) {
    if (pointIndex < 0 || pointIndex >= static_cast<int>(editingKeyframeOrder_.size())) {
        return;
    }
    
    const std::string& pointName = editingKeyframeOrder_[pointIndex];
    
    // JSONから削除
    editingKeyframes_.erase(pointName);
    
    // 順序配列から削除
    editingKeyframeOrder_.erase(editingKeyframeOrder_.begin() + pointIndex);
    
    // 選択インデックスを調整
    if (selectedPointIndex_ >= pointIndex) {
        selectedPointIndex_--;
        if (selectedPointIndex_ < 0 && !editingKeyframeOrder_.empty()) {
            selectedPointIndex_ = 0;
        }
    }
    
    keyframesDirty_ = true;
    
    Log::Send(Log::Level::INFO, std::format("Removed keyframe point '{}'", pointName));
}

void PostProcessPresetEditor::MovePointUp(int pointIndex) {
    if (pointIndex <= 0 || pointIndex >= static_cast<int>(editingKeyframeOrder_.size())) {
        return;
    }
    
    std::swap(editingKeyframeOrder_[pointIndex], editingKeyframeOrder_[pointIndex - 1]);
    
    if (selectedPointIndex_ == pointIndex) {
        selectedPointIndex_ = pointIndex - 1;
    } else if (selectedPointIndex_ == pointIndex - 1) {
        selectedPointIndex_ = pointIndex;
    }
    
    keyframesDirty_ = true;
}

void PostProcessPresetEditor::MovePointDown(int pointIndex) {
    if (pointIndex < 0 || pointIndex >= static_cast<int>(editingKeyframeOrder_.size()) - 1) {
        return;
    }
    
    std::swap(editingKeyframeOrder_[pointIndex], editingKeyframeOrder_[pointIndex + 1]);
    
    if (selectedPointIndex_ == pointIndex) {
        selectedPointIndex_ = pointIndex + 1;
    } else if (selectedPointIndex_ == pointIndex + 1) {
        selectedPointIndex_ = pointIndex;
    }
    
    keyframesDirty_ = true;
}

void PostProcessPresetEditor::ApplyPreset(const std::string& _presetName) {
    if (!executor_) {
        Log::Send(Log::Level::ERR, "Executor not available");
        return;
    }

    if (_presetName.empty()) {
        Log::Send(Log::Level::WARNING, "Cannot apply empty preset name");
        return;
    }

    // ExecutorのApplyPresetを呼び出す
    executor_->ApplyPreset(_presetName);

    Log::Send(Log::Level::INFO, std::format("Applied preset '{}'", _presetName));
}
