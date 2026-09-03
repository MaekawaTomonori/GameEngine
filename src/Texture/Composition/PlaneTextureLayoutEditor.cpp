#include "PlaneTextureLayoutEditor.hpp"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <numeric>

#include "imgui.h"
#include "json.hpp"
#include "DebugUI.hpp"
#include "DebugUIWidgets.hpp"
#include "Log.hpp"
#include "src/Texture/Composition/PlaneTextureBaker.hpp"
#include "src/Texture/TextureManager.hpp"

#undef min
#undef max

namespace {
    /** @brief 移動後の矩形をキャンバス範囲[0,1]内へクランプする（サイズは維持） */
    void ClampMovedRect(PlaneTextureLayer& _layer, const Vector2& _size) {
        const float maxX = std::max(0.f, 1.f - _size.x);
        const float maxY = std::max(0.f, 1.f - _size.y);
        _layer.rectMin.x = std::clamp(_layer.rectMin.x, 0.f, maxX);
        _layer.rectMin.y = std::clamp(_layer.rectMin.y, 0.f, maxY);
        _layer.rectMax.x = _layer.rectMin.x + _size.x;
        _layer.rectMax.y = _layer.rectMin.y + _size.y;
    }

    constexpr float kMinLayerSize = 0.02f;

    constexpr const char* kTemplateDir = "Assets/Data/PlaneTextureLayout/Templates";
    constexpr const char* kPatternRootDir = "Assets/Data/PlaneTextureLayout/Patterns";

    void LayerToJson(nlohmann::json& _j, const PlaneTextureLayer& _layer) {
        _j = nlohmann::json{
            { "type", _layer.type == PlaneTextureLayer::Type::Text ? "Text" : "Texture" },
            { "textureKey", _layer.textureKey },
            { "text", _layer.text },
            { "variableId", _layer.variableId },
            { "rectMin", { _layer.rectMin.x, _layer.rectMin.y } },
            { "rectMax", { _layer.rectMax.x, _layer.rectMax.y } },
            { "color", { _layer.color.x, _layer.color.y, _layer.color.z, _layer.color.w } },
            { "zOrder", _layer.zOrder },
        };
    }

    void LayerFromJson(const nlohmann::json& _j, PlaneTextureLayer& _layer) {
        _layer.type = _j.value("type", std::string("Texture")) == "Text"
            ? PlaneTextureLayer::Type::Text : PlaneTextureLayer::Type::Texture;
        _layer.textureKey = _j.value("textureKey", std::string());
        _layer.text = _j.value("text", std::string());
        _layer.variableId = _j.value("variableId", std::string());

        if (_j.contains("rectMin")) {
            _layer.rectMin.x = _j["rectMin"][0].get<float>();
            _layer.rectMin.y = _j["rectMin"][1].get<float>();
        }
        if (_j.contains("rectMax")) {
            _layer.rectMax.x = _j["rectMax"][0].get<float>();
            _layer.rectMax.y = _j["rectMax"][1].get<float>();
        }
        if (_j.contains("color")) {
            _layer.color.x = _j["color"][0].get<float>();
            _layer.color.y = _j["color"][1].get<float>();
            _layer.color.z = _j["color"][2].get<float>();
            _layer.color.w = _j["color"][3].get<float>();
        }
        _layer.zOrder = _j.value("zOrder", 0);
    }

    struct Pattern {
        std::string outputKey;
        std::unordered_map<std::string, std::string> values;
    };

    bool LoadPatternFile(const std::string& _path, Pattern& _outPattern) {
        std::ifstream in(_path);
        if (!in.is_open()) return false;

        nlohmann::json j;
        in >> j;

        _outPattern.outputKey = j.value("outputKey", std::string());
        _outPattern.values.clear();
        if (j.contains("values")) {
            for (const auto& item : j["values"].items()) {
                _outPattern.values[item.key()] = item.value().get<std::string>();
            }
        }
        return !_outPattern.outputKey.empty();
    }
}

void PlaneTextureLayoutEditor::Initialize(const GESTD::ReferencePtr<DebugUI>& _debug, TextureManager* _texture,
                                           const std::string& _windowName, float _aspectRatio) {
    debug_ = _debug;
    texture_ = _texture;
    windowName_ = _windowName;
    aspectRatio_ = _aspectRatio > 0.f ? _aspectRatio : 1.f;

    if (debug_) {
        debug_->RegisterMenuButton(windowName_);
    }
}

void PlaneTextureLayoutEditor::SetAspectRatio(float _aspectRatio) {
    aspectRatio_ = _aspectRatio > 0.f ? _aspectRatio : 1.f;
}

int PlaneTextureLayoutEditor::AddLayer(const PlaneTextureLayer& _layer) {
    layers_.push_back(_layer);
    return static_cast<int>(layers_.size()) - 1;
}

void PlaneTextureLayoutEditor::RemoveLayer(int _index) {
    if (_index < 0 || _index >= static_cast<int>(layers_.size())) return;
    layers_.erase(layers_.begin() + _index);
    selectedIndex_ = -1;
}

uint64_t PlaneTextureLayoutEditor::ResolveImTextureId(const std::string& _textureKey) {
    if (_textureKey.empty() || !texture_ || !debug_) return 0;

    if (const auto it = textureIdCache_.find(_textureKey); it != textureIdCache_.end()) {
        return it->second;
    }

    // 失敗時も含めてキー単位で1回だけ試す（失敗するキーを毎フレーム再ロードして
    // TextureManager::Load のアラートを連発させないため、結果を必ずキャッシュする）
    texture_->Load(_textureKey);
    ID3D12Resource* resource = texture_->GetResource(_textureKey);
    if (!resource) {
        textureIdCache_[_textureKey] = 0;
        return 0;
    }

    const uint64_t texId = debug_->RegisterTexture(resource, texture_->GetTextureMetadata(_textureKey).format);
    textureIdCache_[_textureKey] = texId;
    return texId;
}

void PlaneTextureLayoutEditor::RefreshAvailableTextures() {
    availableTexturesScanned_ = true;
    availableTextures_ = texture_ ? texture_->ListAvailableTextures() : std::vector<std::string>{};
}

void PlaneTextureLayoutEditor::Debug() {
    if (!debug_) return;

    debug_->RegisterCommand(windowName_, [this]() {
        ImGui::SetNextWindowSize(ImVec2(760.f, 560.f), ImGuiCond_FirstUseEver);
        ImGui::Begin(windowName_.c_str(), &debug_->IsVisible(windowName_));

        // プレビューを左ペインに固定し、右ペイン(リスト/プロパティ)だけが
        // スクロールしてもプレビューが隠れないようにする
        const float previewWidth = std::min(ImGui::GetContentRegionAvail().x * 0.5f, 480.f);
        ImGui::BeginChild("PlaneTextureCanvasPane", ImVec2(previewWidth, 0.f), true);
        RenderCanvas();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("PlaneTextureControlsPane", ImVec2(0.f, 0.f), false);
        RenderTemplateSection();
        RenderLayerList();
        RenderLayerInspector();
        RenderBakeSection();
        ImGui::EndChild();

        ImGui::End();
    });
}

void PlaneTextureLayoutEditor::RenderCanvas() {
    if (layers_.empty()) {
        ImGui::TextDisabled("No layers. Add one below.");
        return;
    }

    const float availWidth = ImGui::GetContentRegionAvail().x;
    const float canvasWidth = std::min(availWidth, 480.f);
    const float canvasHeight = canvasWidth / aspectRatio_;
    const ImVec2 canvasSize(canvasWidth, canvasHeight);
    const ImVec2 origin = ImGui::GetCursorScreenPos();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(origin, ImVec2(origin.x + canvasSize.x, origin.y + canvasSize.y), IM_COL32(35, 35, 35, 255));

    ImGui::Dummy(canvasSize);

    // 背面(zOrder昇順)から描画し、上に重ねていく
    std::vector<int> order(layers_.size());
    std::iota(order.begin(), order.end(), 0);
    std::ranges::sort(order, [this](const int a, const int b) { return layers_[a].zOrder < layers_[b].zOrder; });

    for (const int idx : order) {
        const auto& layer = layers_[idx];
        const ImVec2 rectMinPx(origin.x + layer.rectMin.x * canvasSize.x, origin.y + layer.rectMin.y * canvasSize.y);
        const ImVec2 rectMaxPx(origin.x + layer.rectMax.x * canvasSize.x, origin.y + layer.rectMax.y * canvasSize.y);
        const ImU32 tint = ImGui::ColorConvertFloat4ToU32(ImVec4(layer.color.x, layer.color.y, layer.color.z, layer.color.w));

        if (layer.type == PlaneTextureLayer::Type::Texture) {
            const uint64_t texId = ResolveImTextureId(layer.textureKey);
            if (texId != 0) {
                drawList->AddImage(static_cast<ImTextureID>(texId), rectMinPx, rectMaxPx, ImVec2(0, 0), ImVec2(1, 1), tint);
            } else {
                drawList->AddRectFilled(rectMinPx, rectMaxPx, IM_COL32(80, 40, 40, 200));
            }
        } else {
            drawList->AddRectFilled(rectMinPx, rectMaxPx, IM_COL32(50, 50, 70, 160));
            drawList->AddText(rectMinPx, tint, layer.text.empty() ? "(text)" : layer.text.c_str());
        }

        drawList->AddRect(rectMinPx, rectMaxPx, idx == selectedIndex_ ? IM_COL32(255, 200, 0, 255) : IM_COL32(200, 200, 200, 120));
    }

    // 手前(zOrder降順)からヒットテストし、ドラッグ移動・リサイズを処理する
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        const int idx = *it;
        auto& layer = layers_[idx];
        const ImVec2 rectMinPx(origin.x + layer.rectMin.x * canvasSize.x, origin.y + layer.rectMin.y * canvasSize.y);
        const ImVec2 rectMaxPx(origin.x + layer.rectMax.x * canvasSize.x, origin.y + layer.rectMax.y * canvasSize.y);
        const ImVec2 bodySize(std::max(rectMaxPx.x - rectMinPx.x, 1.f), std::max(rectMaxPx.y - rectMinPx.y, 1.f));

        ImGui::PushID(idx);

        ImGui::SetCursorScreenPos(rectMinPx);
        ImGui::InvisibleButton("body", bodySize);
        if (ImGui::IsItemClicked()) {
            selectedIndex_ = idx;
        }
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            const ImVec2 delta = ImGui::GetIO().MouseDelta;
            const Vector2 size = layer.rectMax - layer.rectMin;
            layer.rectMin += Vector2{ delta.x / canvasSize.x, delta.y / canvasSize.y };
            ClampMovedRect(layer, size);
        }

        // リサイズハンドル（右下角）
        constexpr float kHandle = 8.f;
        ImGui::SetCursorScreenPos(ImVec2(rectMaxPx.x - kHandle * 0.5f, rectMaxPx.y - kHandle * 0.5f));
        ImGui::InvisibleButton("resize", ImVec2(kHandle, kHandle));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            selectedIndex_ = idx;
            const ImVec2 delta = ImGui::GetIO().MouseDelta;
            layer.rectMax.x = std::clamp(layer.rectMax.x + delta.x / canvasSize.x, layer.rectMin.x + kMinLayerSize, 1.f);
            layer.rectMax.y = std::clamp(layer.rectMax.y + delta.y / canvasSize.y, layer.rectMin.y + kMinLayerSize, 1.f);
        }
        drawList->AddRectFilled(ImVec2(rectMaxPx.x - kHandle, rectMaxPx.y - kHandle), rectMaxPx, IM_COL32(255, 200, 0, 220));

        ImGui::PopID();
    }
}

void PlaneTextureLayoutEditor::RenderLayerList() {
    ImGui::Separator();

    if (ImGui::Button("Add Texture Layer")) {
        selectedIndex_ = AddLayer(PlaneTextureLayer{});
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Text Layer")) {
        PlaneTextureLayer layer;
        layer.type = PlaneTextureLayer::Type::Text;
        layer.text = "Text";
        selectedIndex_ = AddLayer(layer);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(selectedIndex_ < 0);
    if (ImGui::Button("Remove Selected")) {
        RemoveLayer(selectedIndex_);
    }
    ImGui::EndDisabled();

    ImGui::BeginChild("PlaneTextureLayerList", ImVec2(0.f, 120.f), true);
    for (int i = 0; std::cmp_less(i, layers_.size()); ++i) {
        const auto& layer = layers_[i];
        const std::string label = layer.type == PlaneTextureLayer::Type::Text
            ? "[" + std::to_string(i) + "] Text: " + (layer.text.empty() ? "(empty)" : layer.text)
            : "[" + std::to_string(i) + "] Texture: " + (layer.textureKey.empty() ? "(none)" : layer.textureKey);

        ImGui::PushID(i);
        if (ImGui::Selectable(label.c_str(), selectedIndex_ == i)) {
            selectedIndex_ = i;
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
}

void PlaneTextureLayoutEditor::RenderLayerInspector() {
    ImGui::Separator();

    if (selectedIndex_ < 0 || std::cmp_greater_equal(selectedIndex_, layers_.size())) {
        ImGui::TextDisabled("No layer selected");
        return;
    }

    auto& layer = layers_[selectedIndex_];

    int typeIndex = layer.type == PlaneTextureLayer::Type::Text ? 1 : 0;
    ImGui::RadioButton("Texture", &typeIndex, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Text", &typeIndex, 1);
    layer.type = typeIndex == 1 ? PlaneTextureLayer::Type::Text : PlaneTextureLayer::Type::Texture;

    char variableIdBuf[128];
    strncpy_s(variableIdBuf, layer.variableId.c_str(), sizeof(variableIdBuf) - 1);
    if (DebugUIWidgets::InputText("Variable ID", variableIdBuf, sizeof(variableIdBuf))) {
        layer.variableId = variableIdBuf;
    }

    if (layer.type == PlaneTextureLayer::Type::Texture) {
        if (!availableTexturesScanned_) {
            RefreshAvailableTextures();
        }

        // 手打ちだとフォルダプレフィックスの有無で失敗しやすいため、
        // Assets/Resources 配下をスキャンした候補から選択させる
        if (ImGui::SmallButton("Refresh")) {
            RefreshAvailableTextures();
        }
        DebugUIWidgets::KeyCombo("Texture Key", availableTextures_, layer.textureKey);
    } else {
        char textBuf[256];
        strncpy_s(textBuf, layer.text.c_str(), sizeof(textBuf) - 1);
        if (DebugUIWidgets::InputText("Content", textBuf, sizeof(textBuf))) {
            layer.text = textBuf;
        }
    }

    DebugUIWidgets::ColorEdit4("Color", &layer.color.x);
    DebugUIWidgets::DragInt("Z Order", &layer.zOrder, 1.f, 0, 0);

    DebugUIWidgets::DragFloat2("Rect Min", &layer.rectMin.x, 0.005f, 0.f, 1.f);
    DebugUIWidgets::DragFloat2("Rect Max", &layer.rectMax.x, 0.005f, 0.f, 1.f);

    layer.rectMax.x = std::max(layer.rectMax.x, layer.rectMin.x + kMinLayerSize);
    layer.rectMax.y = std::max(layer.rectMax.y, layer.rectMin.y + kMinLayerSize);
}

void PlaneTextureLayoutEditor::RefreshAvailableTemplates() {
    availableTemplatesScanned_ = true;
    availableTemplates_.clear();

    namespace fs = std::filesystem;
    std::error_code ec;
    const fs::path dir = kTemplateDir;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) return;

    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file(ec)) continue;
        if (entry.path().extension() != ".json") continue;
        availableTemplates_.push_back(entry.path().stem().string());
    }
    std::sort(availableTemplates_.begin(), availableTemplates_.end());
}

void PlaneTextureLayoutEditor::RefreshPatterns() {
    patternsScanned_ = true;
    patternFiles_.clear();

    namespace fs = std::filesystem;
    std::error_code ec;
    const fs::path dir = fs::path(kPatternRootDir) / templateName_;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) return;

    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file(ec)) continue;
        if (entry.path().extension() != ".json") continue;
        patternFiles_.push_back(entry.path().string());
    }
    std::sort(patternFiles_.begin(), patternFiles_.end());
}

void PlaneTextureLayoutEditor::SaveTemplate(const std::string& _name) {
    if (_name.empty()) return;

    nlohmann::json j;
    j["aspectRatio"] = aspectRatio_;
    j["layers"] = nlohmann::json::array();
    for (const auto& layer : layers_) {
        nlohmann::json layerJson;
        LayerToJson(layerJson, layer);
        j["layers"].push_back(layerJson);
    }

    std::filesystem::create_directories(kTemplateDir);
    std::ofstream out(std::string(kTemplateDir) + "/" + _name + ".json");
    if (!out.is_open()) {
        Log::Send(Log::Level::ERR, std::format("PlaneTextureLayoutEditor: failed to save template {}", _name));
        return;
    }
    out << j.dump(4);

    availableTemplatesScanned_ = false;
}

void PlaneTextureLayoutEditor::LoadTemplate(const std::string& _name) {
    if (_name.empty()) return;

    std::ifstream in(std::string(kTemplateDir) + "/" + _name + ".json");
    if (!in.is_open()) {
        Log::Send(Log::Level::ERR, std::format("PlaneTextureLayoutEditor: failed to load template {}", _name));
        return;
    }

    nlohmann::json j;
    in >> j;

    aspectRatio_ = j.value("aspectRatio", 1.f);
    layers_.clear();
    if (j.contains("layers")) {
        for (const auto& layerJson : j["layers"]) {
            PlaneTextureLayer layer;
            LayerFromJson(layerJson, layer);
            layers_.push_back(layer);
        }
    }
    selectedIndex_ = -1;
    patternsScanned_ = false;
}

std::vector<std::string> PlaneTextureLayoutEditor::GetVariableIds() const {
    std::vector<std::string> ids;
    for (const auto& layer : layers_) {
        if (layer.variableId.empty()) continue;
        if (std::find(ids.begin(), ids.end(), layer.variableId) != ids.end()) continue;
        ids.push_back(layer.variableId);
    }
    return ids;
}

std::vector<PlaneTextureLayer> PlaneTextureLayoutEditor::ResolveLayers(const std::unordered_map<std::string, std::string>& _values) const {
    std::vector<PlaneTextureLayer> resolved = layers_;
    for (auto& layer : resolved) {
        if (layer.variableId.empty()) continue;

        const auto it = _values.find(layer.variableId);
        if (it == _values.end()) continue;

        if (layer.type == PlaneTextureLayer::Type::Texture) {
            layer.textureKey = it->second;
        } else {
            layer.text = it->second;
        }
    }
    return resolved;
}

bool PlaneTextureLayoutEditor::BakeAndRegister(const std::vector<PlaneTextureLayer>& _resolvedLayers, const std::string& _outputKey) {
    if (_outputKey.empty() || !texture_) return false;

    const uint32_t width = static_cast<uint32_t>(std::max(1, outputWidth_));
    const uint32_t height = static_cast<uint32_t>(std::max(1, static_cast<int>(static_cast<float>(width) / aspectRatio_)));

    std::vector<uint8_t> pixels;
    if (!PlaneTextureBaker::Bake(_resolvedLayers, width, height, pixels)) return false;

    return texture_->LoadFromRawPixels(_outputKey, pixels.data(), width, height);
}

void PlaneTextureLayoutEditor::BakeAllPatterns() {
    if (!patternsScanned_) {
        RefreshPatterns();
    }

    for (const auto& path : patternFiles_) {
        Pattern pattern;
        if (!LoadPatternFile(path, pattern)) {
            Log::Send(Log::Level::ERR, std::format("PlaneTextureLayoutEditor: failed to parse pattern {}", path));
            continue;
        }

        const auto resolved = ResolveLayers(pattern.values);
        if (!BakeAndRegister(resolved, pattern.outputKey)) {
            Log::Send(Log::Level::ERR, std::format("PlaneTextureLayoutEditor: failed to bake pattern {}", path));
        }
    }
}

void PlaneTextureLayoutEditor::RenderTemplateSection() {
    ImGui::SeparatorText("Template");

    char nameBuf[128];
    strncpy_s(nameBuf, templateName_.c_str(), sizeof(nameBuf) - 1);
    if (DebugUIWidgets::InputText("Template Name", nameBuf, sizeof(nameBuf))) {
        templateName_ = nameBuf;
        patternsScanned_ = false;
    }

    if (ImGui::Button("Save Template")) {
        SaveTemplate(templateName_);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Refresh##Templates")) {
        RefreshAvailableTemplates();
    }

    if (!availableTemplatesScanned_) {
        RefreshAvailableTemplates();
    }

    std::string loadTarget = templateName_;
    if (DebugUIWidgets::KeyCombo("Load Template", availableTemplates_, loadTarget) && !loadTarget.empty()) {
        templateName_ = loadTarget;
        LoadTemplate(loadTarget);
    }
}

void PlaneTextureLayoutEditor::RenderBakeSection() {
    ImGui::SeparatorText("Bake");

    DebugUIWidgets::DragInt("Output Width", &outputWidth_, 1.f, 16, 4096);
    ImGui::Text("Output Height: %d", std::max(1, static_cast<int>(static_cast<float>(outputWidth_) / aspectRatio_)));

    ImGui::SeparatorText("Bake (single)");

    const auto variableIds = GetVariableIds();
    if (variableIds.empty()) {
        ImGui::TextDisabled("No variable layers (set Variable ID on a layer to use this)");
    } else {
        if (!availableTexturesScanned_) {
            RefreshAvailableTextures();
        }

        for (const auto& id : variableIds) {
            PlaneTextureLayer::Type varType = PlaneTextureLayer::Type::Texture;
            for (const auto& layer : layers_) {
                if (layer.variableId != id) continue;
                varType = layer.type;
                break;
            }

            std::string& value = singleBakeValues_[id];
            ImGui::PushID(id.c_str());
            if (varType == PlaneTextureLayer::Type::Texture) {
                DebugUIWidgets::KeyCombo(id.c_str(), availableTextures_, value);
            } else {
                char buf[256];
                strncpy_s(buf, value.c_str(), sizeof(buf) - 1);
                if (DebugUIWidgets::InputText(id.c_str(), buf, sizeof(buf))) {
                    value = buf;
                }
            }
            ImGui::PopID();
        }
    }

    char outputKeyBuf[128];
    strncpy_s(outputKeyBuf, singleBakeOutputKey_.c_str(), sizeof(outputKeyBuf) - 1);
    if (DebugUIWidgets::InputText("Output Texture Key", outputKeyBuf, sizeof(outputKeyBuf))) {
        singleBakeOutputKey_ = outputKeyBuf;
    }

    if (ImGui::Button("Bake")) {
        const auto resolved = ResolveLayers(singleBakeValues_);
        if (!BakeAndRegister(resolved, singleBakeOutputKey_)) {
            Log::Send(Log::Level::ERR, std::format("PlaneTextureLayoutEditor: bake failed for {}", singleBakeOutputKey_));
        }
    }

    ImGui::SeparatorText("Patterns (batch)");
    ImGui::TextDisabled("%s", (std::string(kPatternRootDir) + "/" + templateName_).c_str());

    if (ImGui::SmallButton("Scan Patterns")) {
        RefreshPatterns();
    }
    if (!patternsScanned_) {
        RefreshPatterns();
    }

    ImGui::BeginChild("PlaneTexturePatternList", ImVec2(0.f, 80.f), true);
    for (const auto& path : patternFiles_) {
        ImGui::TextUnformatted(std::filesystem::path(path).filename().string().c_str());
    }
    ImGui::EndChild();

    ImGui::BeginDisabled(patternFiles_.empty());
    if (ImGui::Button("Bake All Patterns")) {
        BakeAllPatterns();
    }
    ImGui::EndDisabled();
}
