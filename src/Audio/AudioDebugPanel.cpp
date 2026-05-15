#ifdef _DEBUG
#include "AudioDebugPanel.hpp"

#include <algorithm>
#include <cmath>

#include "imgui_internal.h"
#include "AudioLib.hpp"
#include "Utils.hpp"

#undef min
#undef max

// ================================================================
//  DAW visual constants (translation-unit local)
// ================================================================
namespace {
namespace DawStyle {

    const ImVec4 kChannelColors[] = {
        { 0.18f, 0.52f, 0.62f, 1.f },  // teal
        { 0.72f, 0.40f, 0.12f, 1.f },  // orange
        { 0.52f, 0.28f, 0.68f, 1.f },  // purple
        { 0.22f, 0.58f, 0.28f, 1.f },  // green
        { 0.62f, 0.22f, 0.22f, 1.f },  // red
        { 0.28f, 0.38f, 0.68f, 1.f },  // blue
    };
    constexpr int   kColorCount   = 6;
    constexpr float kChannelWidth = 90.f;
    constexpr float kMasterWidth  = 104.f;
    constexpr float kFaderHeight  = 130.f;
    constexpr float kVUWidth      = 10.f;
    constexpr float kVUHeight     = 90.f;
    constexpr float kHeaderHeight = 26.f;
    constexpr float kAddTrackWidth= 44.f;

    const ImVec4 kTextDim      = { 0.50f, 0.50f, 0.50f, 1.f };
    const ImVec4 kMuteActive   = { 0.85f, 0.25f, 0.25f, 1.f };
    const ImVec4 kLoopActive   = { 0.25f, 0.55f, 0.85f, 1.f };
    const ImVec4 kPlayActive   = { 0.22f, 0.72f, 0.38f, 1.f };
    const ImVec4 kPauseActive  = { 0.72f, 0.60f, 0.10f, 1.f };
    const ImVec4 kStopAll      = { 0.55f, 0.25f, 0.08f, 1.f };
    const ImVec4 kStopAllHover = { 0.72f, 0.33f, 0.10f, 1.f };

} // namespace DawStyle

// ================================================================
//  VU meter helpers
// ================================================================
void DrawVUBar(ImDrawList* dl, float x, float y, float w, float h, float level) {
    dl->AddRectFilled({ x, y }, { x + w, y + h }, IM_COL32(20, 20, 20, 255), 2.f);
    if (level <= 0.001f) return;

    float fillH   = h * std::min(level, 1.0f);
    float greenH  = h * 0.75f;
    float yellowH = h * 0.15f;
    float greenTop  = y + h - greenH;
    float yellowTop = greenTop - yellowH;

    if (fillH <= greenH) {
        dl->AddRectFilled({ x, y + h - fillH }, { x + w, y + h }, IM_COL32(40, 200, 60, 255), 2.f);
    } else if (fillH <= greenH + yellowH) {
        float yf = fillH - greenH;
        dl->AddRectFilled({ x, greenTop },      { x + w, y + h },    IM_COL32(40, 200, 60,  255), 2.f);
        dl->AddRectFilled({ x, greenTop - yf }, { x + w, greenTop }, IM_COL32(220, 200, 30, 255), 2.f);
    } else {
        float rf = fillH - greenH - yellowH;
        dl->AddRectFilled({ x, greenTop },       { x + w, y + h },    IM_COL32(40, 200, 60,  255), 2.f);
        dl->AddRectFilled({ x, yellowTop },      { x + w, greenTop }, IM_COL32(220, 200, 30, 255), 2.f);
        dl->AddRectFilled({ x, yellowTop - rf }, { x + w, yellowTop}, IM_COL32(220, 60,  40, 255), 2.f);
    }
}

float SimulateVU(int seed, float time) {
    float f1 = fabsf(sinf(time *  8.5f + static_cast<float>(seed) * 2.1f));
    float f2 = fabsf(sinf(time * 17.3f + static_cast<float>(seed) * 0.9f));
    float f3 = fabsf(sinf(time * 33.7f + static_cast<float>(seed) * 1.5f));
    return std::clamp(0.55f * f1 + 0.30f * f2 + 0.15f * f3, 0.0f, 1.0f);
}

} // anonymous namespace

// ================================================================
//  Public interface
// ================================================================
void AudioDebugPanel::Initialize(const GESTD::ReferencePtr<DebugUI>& _debugUI) {
    debugUI_ = _debugUI;
    if (!debugUI_) {
        Utils::Alert("AudioDebugPanel: DebugUI not registered");
        return;
    }

    debugUI_->RegisterMenuButton("Audio Loader", false, "Audio");
    debugUI_->RegisterMenuButton("Audio Player", false, "Audio");
    debugUI_->RegisterMenuButton("Audio Mixer",  false, "Audio");
}

void AudioDebugPanel::Debug() {
    if (!debugUI_) return;

    debugUI_->RegisterCommand("Audio Loader", [this]() {
        ImGui::Begin("Audio Loader", &debugUI_->IsVisible("Audio Loader"));
        DrawLoaderWindow();
        ImGui::End();
    });

    debugUI_->RegisterCommand("Audio Player", [this]() {
        ImGui::Begin("Audio Player", &debugUI_->IsVisible("Audio Player"));
        DrawPlayerWindow();
        ImGui::End();
    });

    debugUI_->RegisterCommand("Audio Mixer", [this]() {
        ImGui::Begin("Audio Mixer", &debugUI_->IsVisible("Audio Mixer"));
        DrawMixerWindow();
        ImGui::End();
    });
}

// ================================================================
//  Track management
// ================================================================
void AudioDebugPanel::AddTrack() {
    Track t;
    t.name     = "Track " + std::to_string(tracks_.size() + 1);
    t.handle   = Audio::CreateTrack();
    t.colorIdx = nextColorIdx_;
    nextColorIdx_ = (nextColorIdx_ + 1) % DawStyle::kColorCount;
    tracks_.push_back(std::move(t));
    focusedTrack_ = static_cast<int>(tracks_.size()) - 1;
    if (targetTrack_ < 0) targetTrack_ = 0;
}

void AudioDebugPanel::RemoveTrack(int idx) {
    Audio::DestroyTrack(tracks_[idx].handle);
    tracks_.erase(tracks_.begin() + idx);

    if (focusedTrack_ == idx)      focusedTrack_ = -1;
    else if (focusedTrack_ > idx)  --focusedTrack_;

    if (targetTrack_ == idx)       targetTrack_ = tracks_.empty() ? -1 : 0;
    else if (targetTrack_ > idx)   --targetTrack_;
}

// ================================================================
//  Loader window — load audio assets and list them
// ================================================================
void AudioDebugPanel::DrawLoaderWindow() {
    ImGui::SetNextItemWidth(220.f);
    ImGui::InputText("##path", pathBuf_, sizeof(pathBuf_));
    ImGui::SameLine(0, 6.f);

    ImGui::PushStyleColor(ImGuiCol_Button,        { 0.18f, 0.42f, 0.18f, 1.f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.24f, 0.55f, 0.24f, 1.f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.12f, 0.30f, 0.12f, 1.f });
    if (ImGui::Button("Load") && pathBuf_[0] != '\0') {
        loadedSounds_.push_back({ pathBuf_, Audio::Handle(pathBuf_), nextColorIdx_ });
        nextColorIdx_  = (nextColorIdx_ + 1) % DawStyle::kColorCount;
        selectedSound_ = static_cast<int>(loadedSounds_.size()) - 1;
    }
    ImGui::PopStyleColor(3);

    ImGui::Separator();
    ImGui::BeginChild("##SoundList", { 0.f, 0.f }, false);

    int removeIdx = -1;
    for (int i = 0; i < static_cast<int>(loadedSounds_.size()); ++i) {
        ImGui::PushID(i);
        const ImVec4& col = DawStyle::kChannelColors[loadedSounds_[i].colorIdx];
        ImGui::PushStyleColor(ImGuiCol_Header,        col);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { col.x + 0.1f, col.y + 0.1f, col.z + 0.1f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  { col.x - 0.1f, col.y - 0.1f, col.z - 0.1f, 1.f });
        const bool selected = (selectedSound_ == i);
        if (ImGui::Selectable(loadedSounds_[i].path.c_str(), selected,
            ImGuiSelectableFlags_None, { ImGui::GetContentRegionAvail().x - 30.f, 0.f }))
            selectedSound_ = i;
        ImGui::PopStyleColor(3);

        ImGui::SameLine(0, 4.f);
        ImGui::PushStyleColor(ImGuiCol_Button,        { 0.35f, 0.14f, 0.14f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.55f, 0.18f, 0.18f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.22f, 0.08f, 0.08f, 1.f });
        if (ImGui::SmallButton(" x ")) removeIdx = i;
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }

    if (removeIdx >= 0) {
        loadedSounds_.erase(loadedSounds_.begin() + removeIdx);
        if (selectedSound_ == removeIdx)      selectedSound_ = -1;
        else if (selectedSound_ > removeIdx)  --selectedSound_;
    }

    ImGui::EndChild();
}

// ================================================================
//  Player window — choose sound, target track, params, then play
// ================================================================
void AudioDebugPanel::DrawPlayerWindow() {
    const bool hasSelection = (selectedSound_ >= 0 &&
                               selectedSound_ < static_cast<int>(loadedSounds_.size()));

    if (hasSelection) {
        ImGui::PushStyleColor(ImGuiCol_Text, DawStyle::kChannelColors[loadedSounds_[selectedSound_].colorIdx]);
        ImGui::TextUnformatted(loadedSounds_[selectedSound_].path.c_str());
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, DawStyle::kTextDim);
        ImGui::TextUnformatted("-- No sound selected (click a row in Audio Loader) --");
        ImGui::PopStyleColor();
    }

    ImGui::Separator();
    ImGui::BeginDisabled(!hasSelection);

    // Target track selector
    ImGui::SetNextItemWidth(260.f);
    {
        const char* preview = (targetTrack_ < 0) ? "MASTER"
                                                  : tracks_[targetTrack_].name.c_str();
        if (ImGui::BeginCombo("Target Track", preview)) {
            if (ImGui::Selectable("MASTER", targetTrack_ < 0)) targetTrack_ = -1;
            if (targetTrack_ < 0) ImGui::SetItemDefaultFocus();
            for (int i = 0; i < static_cast<int>(tracks_.size()); ++i) {
                const bool sel = (i == targetTrack_);
                if (ImGui::Selectable(tracks_[i].name.c_str(), sel)) targetTrack_ = i;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    ImGui::Spacing();

    ImGui::SetNextItemWidth(260.f);
    ImGui::SliderFloat("Volume##p", &playerVolume_, 0.f, 1.f,   "%.2f");
    ImGui::SetNextItemWidth(260.f);
    ImGui::SliderFloat("Pitch##p",  &playerPitch_,  0.1f, 3.f,  "%.2fx");
    ImGui::SetNextItemWidth(260.f);
    ImGui::SliderFloat("Pan##p",    &playerPan_,    -1.f, 1.f,  "%.2f");

    // Loop toggle
    {
        const bool wasLoop = playerLoop_;
        if (wasLoop) {
            ImGui::PushStyleColor(ImGuiCol_Button,        DawStyle::kLoopActive);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.40f, 0.70f, 1.f, 1.f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.10f, 0.35f, 0.7f, 1.f });
        }
        if (ImGui::Button(playerLoop_ ? "Loop: ON " : "Loop: OFF"))
            playerLoop_ = !playerLoop_;
        if (wasLoop) ImGui::PopStyleColor(3);
    }

    ImGui::SameLine(0, 12.f);

    // PLAY button
    ImGui::PushStyleColor(ImGuiCol_Button,        DawStyle::kPlayActive);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.35f, 0.90f, 0.50f, 1.f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.12f, 0.50f, 0.22f, 1.f });
    if (ImGui::Button("  >  PLAY  ") && hasSelection) {
        auto& sound = loadedSounds_[selectedSound_];

        PlayingItem item;
        item.soundName = sound.path;
        item.volume    = playerVolume_;
        item.pitch     = playerPitch_;
        item.pan       = playerPan_;
        item.loop      = playerLoop_;

        if (targetTrack_ >= 0 && targetTrack_ < static_cast<int>(tracks_.size())) {
            auto& track   = tracks_[targetTrack_];
            item.playback = sound.handle.Play(playerVolume_, playerPitch_, playerPan_, playerLoop_, track.handle);
            track.items.push_back(std::move(item));
            focusedTrack_ = targetTrack_;
        } else {
            item.playback = sound.handle.Play(playerVolume_, playerPitch_, playerPan_, playerLoop_);
            masterItems_.push_back(std::move(item));
            focusedTrack_ = -1;
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::EndDisabled();
}

// ================================================================
//  Mixer window — Master + Track strips left, playback list right
// ================================================================
void AudioDebugPanel::DrawMixerWindow() {
    constexpr float kSplitterW  = 6.f;
    const float     availWidth  = ImGui::GetContentRegionAvail().x;
    listPanelWidth_ = std::clamp(listPanelWidth_, 160.f, availWidth - 200.f);
    const float stripPanelWidth = availWidth - listPanelWidth_ - kSplitterW;

    // Left: strip panel
    ImGui::BeginChild("##Strips", { stripPanelWidth, 0.f }, false,
        ImGuiWindowFlags_HorizontalScrollbar);

    float gx = ImGui::GetCursorPosX();
    DrawMasterStrip(gx);
    ImGui::SameLine(0, 0.f);

    ImGui::PushStyleColor(ImGuiCol_Separator, { 0.32f, 0.32f, 0.32f, 1.f });
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 8.f);

    int removeIdx = -1;
    for (int i = 0; i < static_cast<int>(tracks_.size()); ++i) {
        bool remove = false;
        DrawTrackStrip(tracks_[i], i, ImGui::GetCursorPosX(), remove);
        if (remove) removeIdx = i;
        ImGui::SameLine(0, 8.f);
    }
    if (removeIdx >= 0) RemoveTrack(removeIdx);

    // Add Track button
    {
        const float btnH = ImGui::GetContentRegionAvail().y;
        ImGui::PushStyleColor(ImGuiCol_Button,        { 0.12f, 0.20f, 0.12f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.18f, 0.32f, 0.18f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.08f, 0.14f, 0.08f, 1.f });
        if (ImGui::Button("+\nT\nr\na\nc\nk", { DawStyle::kAddTrackWidth, btnH }))
            AddTrack();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Add Track");
        ImGui::PopStyleColor(3);
    }
    ImGui::EndChild();

    // Splitter
    ImGui::SameLine(0, 0.f);
    {
        const float splitterH = ImGui::GetContentRegionAvail().y;
        ImGui::InvisibleButton("##splitter", { kSplitterW, splitterH });
        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        if (ImGui::IsItemActive())
            listPanelWidth_ -= ImGui::GetIO().MouseDelta.x;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        const ImU32 col = (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ? IM_COL32(120, 120, 120, 255)
            : IM_COL32(50,  50,  50,  255);
        dl->AddLine({ p.x - kSplitterW * 0.5f, p.y },
                    { p.x - kSplitterW * 0.5f, p.y + splitterH }, col, 1.f);
    }
    ImGui::SameLine(0, 0.f);

    // Right: playback list
    ImGui::BeginChild("##PlaybackPanel", { listPanelWidth_, 0.f }, false,
        ImGuiWindowFlags_HorizontalScrollbar);
    if (focusedTrack_ >= 0 && focusedTrack_ < static_cast<int>(tracks_.size())) {
        auto& t = tracks_[focusedTrack_];
        DrawPlaybackList(t.name, DawStyle::kChannelColors[t.colorIdx], t.items);
    } else {
        DrawPlaybackList("MASTER", { 0.75f, 0.75f, 0.75f, 1.f }, masterItems_);
    }
    ImGui::EndChild();
}

// ================================================================
//  Dual VU meter
// ================================================================
void AudioDebugPanel::DrawDualVU(int seed, float volume, bool active, float stripWidth) {
    float time   = static_cast<float>(ImGui::GetTime());
    float levelL = active ? SimulateVU(seed,     time) * volume : 0.f;
    float levelR = active ? SimulateVU(seed + 7, time) * volume : 0.f;

    ImDrawList* dl  = ImGui::GetWindowDrawList();
    ImVec2      pos = ImGui::GetCursorScreenPos();
    float gap  = 3.f;
    float totW = DawStyle::kVUWidth * 2.f + gap;
    float offX = (stripWidth - totW) * 0.5f;
    DrawVUBar(dl, pos.x + offX,                             pos.y, DawStyle::kVUWidth, DawStyle::kVUHeight, levelL);
    DrawVUBar(dl, pos.x + offX + DawStyle::kVUWidth + gap,  pos.y, DawStyle::kVUWidth, DawStyle::kVUHeight, levelR);
    ImGui::Dummy({ stripWidth, DawStyle::kVUHeight });
}

// ================================================================
//  Master strip
// ================================================================
void AudioDebugPanel::DrawMasterStrip(float groupX) {
    masterItems_.erase(std::remove_if(masterItems_.begin(), masterItems_.end(),
        [](const PlayingItem& item) { return !item.playback.IsPlaying() && !item.paused; }),
        masterItems_.end());

    bool anyActive = false;
    for (auto& item : masterItems_)
        if (item.playback.IsPlaying() && !item.paused) { anyActive = true; break; }
    if (!anyActive) {
        for (auto& t : tracks_)
            for (auto& item : t.items)
                if (item.playback.IsPlaying() && !item.paused) { anyActive = true; break; }
    }

    const bool focused = (focusedTrack_ < 0);

    ImGui::BeginGroup();

    // Header
    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        float w = DawStyle::kMasterWidth, h = DawStyle::kHeaderHeight;
        const ImU32 hdrCol = focused ? IM_COL32(110, 110, 110, 255) : IM_COL32(70, 70, 70, 255);
        dl->AddRectFilled(pos, { pos.x + w, pos.y + h }, hdrCol, 4.f, ImDrawFlags_RoundCornersTop);
        if (focused)
            dl->AddCircleFilled({ pos.x + 8.f, pos.y + h * 0.5f }, 3.f, IM_COL32(255, 255, 255, 210));
        const char* lbl = "MASTER";
        ImVec2 tsz = ImGui::CalcTextSize(lbl);
        dl->AddText({ pos.x + (w - tsz.x) * 0.5f, pos.y + (h - tsz.y) * 0.5f }, IM_COL32(230, 230, 230, 255), lbl);
        ImGui::InvisibleButton("##master_hdr", { w, h });
        if (ImGui::IsItemClicked()) focusedTrack_ = -1;
    }

    // LED
    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        float r = 5.f;
        ImVec2 ctr = { pos.x + DawStyle::kMasterWidth * 0.5f, pos.y + r + 3.f };
        ImU32 col = anyActive ? IM_COL32(50, 255, 80, 255) : IM_COL32(28, 60, 28, 255);
        dl->AddCircleFilled(ctr, r, col);
        dl->AddCircle(ctr, r, IM_COL32(0, 0, 0, 160), 16, 1.2f);
        ImGui::Dummy({ DawStyle::kMasterWidth, r * 2.f + 6.f });
    }

    DrawDualVU(99, masterVolume_, anyActive, DawStyle::kMasterWidth);

    // Volume fader
    {
        float fw = 40.f;
        ImGui::SetCursorPosX(groupX + (DawStyle::kMasterWidth - fw) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,         { 0.06f, 0.06f, 0.06f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_SliderGrab,       { 0.82f, 0.82f, 0.82f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, { 1.00f, 1.00f, 1.00f, 1.f });
        if (ImGui::VSliderFloat("##master", { fw, DawStyle::kFaderHeight }, &masterVolume_, 0.f, 1.f, ""))
            Audio::SetMasterVolume(masterVolume_);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Master: %.2f", masterVolume_);
        ImGui::PopStyleColor(3);
    }

    // Readout
    {
        char buf[12]; snprintf(buf, sizeof(buf), "%.2f", masterVolume_);
        ImGui::SetCursorPosX(groupX + (DawStyle::kMasterWidth - ImGui::CalcTextSize(buf).x) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, DawStyle::kTextDim);
        ImGui::TextUnformatted(buf);
        ImGui::PopStyleColor();
    }

    ImGui::EndGroup();
}

// ================================================================
//  Track strip
// ================================================================
void AudioDebugPanel::DrawTrackStrip(Track& t, int idx, float groupX, bool& remove) {
    t.items.erase(std::remove_if(t.items.begin(), t.items.end(),
        [](const PlayingItem& item) { return !item.playback.IsPlaying() && !item.paused; }),
        t.items.end());

    const bool    focused = (idx == focusedTrack_);
    const ImVec4& baseCol = DawStyle::kChannelColors[t.colorIdx];
    const ImVec4  hdrCol  = focused
        ? ImVec4(std::min(baseCol.x + 0.18f, 1.f), std::min(baseCol.y + 0.18f, 1.f),
                 std::min(baseCol.z + 0.18f, 1.f), 1.f)
        : baseCol;

    bool anyActive = false;
    for (auto& item : t.items)
        if (item.playback.IsPlaying() && !item.paused) { anyActive = true; break; }

    ImGui::PushID(idx);
    ImGui::BeginGroup();

    // Colored header
    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        float w = DawStyle::kChannelWidth, h = DawStyle::kHeaderHeight;
        dl->AddRectFilled(pos, { pos.x + w, pos.y + h },
            ImGui::ColorConvertFloat4ToU32(hdrCol), 4.f, ImDrawFlags_RoundCornersTop);
        if (focused)
            dl->AddCircleFilled({ pos.x + 8.f, pos.y + h * 0.5f }, 3.f, IM_COL32(255, 255, 255, 210));
        std::string lbl = t.name;
        if (lbl.size() > 9) lbl = lbl.substr(0, 8) + "~";
        ImVec2 tsz = ImGui::CalcTextSize(lbl.c_str());
        dl->AddText({ pos.x + (w - tsz.x) * 0.5f, pos.y + (h - tsz.y) * 0.5f },
            IM_COL32(230, 230, 230, 255), lbl.c_str());
        ImGui::InvisibleButton("##hdr", { w, h });
        if (ImGui::IsItemClicked()) focusedTrack_ = idx;
    }

    // LED
    {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        float r = 5.f;
        ImVec2 ctr = { pos.x + DawStyle::kChannelWidth * 0.5f, pos.y + r + 3.f };
        ImU32 col = anyActive ? IM_COL32(50, 255, 80, 255) : IM_COL32(28, 60, 28, 255);
        dl->AddCircleFilled(ctr, r, col);
        dl->AddCircle(ctr, r, IM_COL32(0, 0, 0, 160), 16, 1.2f);
        ImGui::Dummy({ DawStyle::kChannelWidth, r * 2.f + 6.f });
    }

    DrawDualVU(idx, t.volume, anyActive, DawStyle::kChannelWidth);

    // Volume fader
    {
        float fw = 34.f;
        ImGui::SetCursorPosX(groupX + (DawStyle::kChannelWidth - fw) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,         { 0.06f, 0.06f, 0.06f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_SliderGrab,       { 0.72f, 0.72f, 0.72f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, { 1.00f, 1.00f, 1.00f, 1.f });
        if (ImGui::VSliderFloat("##vol", { fw, DawStyle::kFaderHeight }, &t.volume, 0.f, 1.f, ""))
            t.handle.SetVolume(t.volume);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Track Volume: %.2f", t.volume);
        ImGui::PopStyleColor(3);
    }

    // Volume readout
    {
        char buf[12]; snprintf(buf, sizeof(buf), "%.2f", t.volume);
        ImGui::SetCursorPosX(groupX + (DawStyle::kChannelWidth - ImGui::CalcTextSize(buf).x) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, DawStyle::kTextDim);
        ImGui::TextUnformatted(buf);
        ImGui::PopStyleColor();
    }

    // Pan
    ImGui::PushStyleColor(ImGuiCol_Text, DawStyle::kTextDim);
    ImGui::SetCursorPosX(groupX);
    ImGui::TextUnformatted("PAN");
    ImGui::PopStyleColor();
    ImGui::SetCursorPosX(groupX);
    ImGui::SetNextItemWidth(DawStyle::kChannelWidth);
    if (ImGui::SliderFloat("##pan", &t.pan, -1.f, 1.f, "%.2f"))
        t.handle.SetPan(t.pan);

    // Pitch
    ImGui::PushStyleColor(ImGuiCol_Text, DawStyle::kTextDim);
    ImGui::SetCursorPosX(groupX);
    ImGui::TextUnformatted("PITCH");
    ImGui::PopStyleColor();
    ImGui::SetCursorPosX(groupX);
    ImGui::SetNextItemWidth(DawStyle::kChannelWidth);
    if (ImGui::SliderFloat("##pit", &t.pitch, 0.1f, 3.f, "%.2fx"))
        t.handle.SetPitch(t.pitch);

    ImGui::Spacing();

    // Stop All
    ImGui::SetCursorPosX(groupX);
    ImGui::PushStyleColor(ImGuiCol_Button,        DawStyle::kStopAll);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, DawStyle::kStopAllHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.35f, 0.15f, 0.04f, 1.f });
    if (ImGui::Button("Stop All", { DawStyle::kChannelWidth, 0.f })) {
        t.handle.StopAll();
        for (auto& item : t.items) item.paused = false;
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();

    // Remove track
    ImGui::SetCursorPosX(groupX);
    ImGui::PushStyleColor(ImGuiCol_Button,        { 0.22f, 0.10f, 0.10f, 1.f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.45f, 0.16f, 0.16f, 1.f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.15f, 0.06f, 0.06f, 1.f });
    if (ImGui::Button("Remove", { DawStyle::kChannelWidth, 0.f }))
        remove = true;
    ImGui::PopStyleColor(3);

    ImGui::EndGroup();
    ImGui::PopID();
}

// ================================================================
//  Playback list (right panel of Mixer)
// ================================================================
void AudioDebugPanel::DrawPlaybackList(std::string_view name, ImVec4 headerColor,
                                       std::vector<PlayingItem>& items) {
    {
        ImGui::PushStyleColor(ImGuiCol_Text, headerColor);
        ImGui::TextUnformatted(name.data());
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, DawStyle::kTextDim);
        ImGui::Text("(%d active)", static_cast<int>(items.size()));
        ImGui::PopStyleColor();
    }
    ImGui::Separator();

    if (items.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, DawStyle::kTextDim);
        ImGui::TextUnformatted("No active playbacks");
        ImGui::PopStyleColor();
        return;
    }

    constexpr ImGuiTableFlags kTableFlags =
        ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (!ImGui::BeginTable("##items", 7, kTableFlags)) return;

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Sound",  ImGuiTableColumnFlags_WidthFixed, 82.f);
    ImGui::TableSetupColumn("Vol",    ImGuiTableColumnFlags_WidthFixed, 72.f);
    ImGui::TableSetupColumn("Pan",    ImGuiTableColumnFlags_WidthFixed, 72.f);
    ImGui::TableSetupColumn("M",      ImGuiTableColumnFlags_WidthFixed, 22.f);
    ImGui::TableSetupColumn("L",      ImGuiTableColumnFlags_WidthFixed, 22.f);
    ImGui::TableSetupColumn("Trans",  ImGuiTableColumnFlags_WidthFixed, 58.f);
    ImGui::TableSetupColumn("##led",  ImGuiTableColumnFlags_WidthFixed, 14.f);
    ImGui::TableHeadersRow();

    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        PlayingItem& item    = items[i];
        const bool   playing = item.playback.IsPlaying();

        ImGui::PushID(i);
        ImGui::TableNextRow();

        // Sound name
        ImGui::TableSetColumnIndex(0);
        std::string lbl = item.soundName;
        const size_t sl = lbl.find_last_of("/\\");
        if (sl != std::string::npos) lbl = lbl.substr(sl + 1);
        if (lbl.size() > 9) lbl = lbl.substr(0, 8) + "~";
        ImGui::TextUnformatted(lbl.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", item.soundName.c_str());

        // Volume
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(68.f);
        if (ImGui::SliderFloat("##vol", &item.volume, 0.f, 1.f, "%.2f"))
            item.playback.SetVolume(item.volume);

        // Pan
        ImGui::TableSetColumnIndex(2);
        ImGui::SetNextItemWidth(68.f);
        if (ImGui::SliderFloat("##pan", &item.pan, -1.f, 1.f, "%.2f"))
            item.playback.SetPan(item.pan);

        // Mute
        ImGui::TableSetColumnIndex(3);
        {
            const bool wasMuted = item.muted;
            if (wasMuted) {
                ImGui::PushStyleColor(ImGuiCol_Button,        DawStyle::kMuteActive);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1.f, 0.4f, 0.4f, 1.f });
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.6f, 0.1f, 0.1f, 1.f });
            }
            if (ImGui::SmallButton("M")) { item.muted = !item.muted; item.playback.Mute(item.muted); }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Mute");
            if (wasMuted) ImGui::PopStyleColor(3);
        }

        // Loop
        ImGui::TableSetColumnIndex(4);
        {
            const bool wasLoop = item.loop;
            if (wasLoop) {
                ImGui::PushStyleColor(ImGuiCol_Button,        DawStyle::kLoopActive);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.4f, 0.7f, 1.f, 1.f });
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.1f, 0.35f, 0.7f, 1.f });
            }
            if (ImGui::SmallButton("L")) { item.loop = !item.loop; item.playback.SetLoop(item.loop); }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Loop");
            if (wasLoop) ImGui::PopStyleColor(3);
        }

        // Transport: Pause/Resume | Stop
        ImGui::TableSetColumnIndex(5);
        {
            const bool wasPaused = item.paused;
            if (wasPaused) {
                ImGui::PushStyleColor(ImGuiCol_Button,        DawStyle::kPauseActive);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.78f, 0.18f, 1.f });
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.5f, 0.40f, 0.05f, 1.f });
            }
            if (ImGui::SmallButton("||")) {
                if (playing && !item.paused)  { item.playback.Pause();  item.paused = true;  }
                else if (item.paused)          { item.playback.Resume(); item.paused = false; }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pause / Resume");
            if (wasPaused) ImGui::PopStyleColor(3);

            ImGui::SameLine(0, 3.f);

            if (ImGui::SmallButton("[]")) { item.playback.Stop(); item.paused = false; }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Stop");
        }

        // Playback state LED
        ImGui::TableSetColumnIndex(6);
        {
            ImDrawList* dl  = ImGui::GetWindowDrawList();
            ImVec2      pos = ImGui::GetCursorScreenPos();
            float r = 5.f;
            ImVec2 ctr = { pos.x + r, pos.y + r + 3.f };
            ImU32 col = (playing && !item.paused) ? IM_COL32(50, 255, 80,  255)
                       : item.paused              ? IM_COL32(220, 190, 30, 255)
                                                  : IM_COL32(28, 60, 28,   255);
            dl->AddCircleFilled(ctr, r, col);
            dl->AddCircle(ctr, r, IM_COL32(0, 0, 0, 160), 12, 1.f);
            ImGui::Dummy({ r * 2.f, r * 2.f + 6.f });
        }

        ImGui::PopID();
    }

    ImGui::EndTable();
}

#endif // _DEBUG
