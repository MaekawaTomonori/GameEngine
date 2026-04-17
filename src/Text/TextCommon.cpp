#define STB_TRUETYPE_IMPLEMENTATION
#include "font/stb_truetype.h"

#include "TextCommon.hpp"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "json.hpp"

#include "Text/Text.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"

#include "Log.hpp"
#include "Utils.hpp"
#include "src/DirectX/RootSignature/BlendMode.hpp"
#include "src/DirectX/RootSignature/RootSignature.hpp"
#include "src/DirectX/Shader/Shader.h"
#include "src/Renderer/Renderer.hpp"
#include "src/Texture/TextureManager.hpp"

#ifdef _DEBUG
#include "imgui.h"
#endif

#undef min
#undef max

void TextCommon::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi, TextureManager* _texture, SRVManager* _srv) {
    texture_ = _texture;
    srv_     = _srv;

    if (!_texture || !_srv) {
        Utils::Alert("TextCommon: required dependency is null");
        return;
    }

    Setup(_adapter, _debugUi, "Text Editor");

    LoadFromJson();
    SetupPipeline();

    instanceBuffer_ = adapter_->CreateBufferResource(sizeof(GlyphInstance) * kMaxGlyphs);
    if (!instanceBuffer_) {
        Utils::Alert("TextCommon: failed to create instance buffer");
        return;
    }
    instanceBuffer_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedInstances_));

    glyphSrvIndex_ = srv_->Allocate();
    srv_->CreateSRVForStructuredBuffer(
        glyphSrvIndex_,
        instanceBuffer_->Get(),
        kMaxGlyphs,
        sizeof(GlyphInstance));
    glyphGpuHandle_ = srv_->GetGPUHandle(glyphSrvIndex_);

    screenParamsCb_ = adapter_->CreateBufferResource((sizeof(ScreenParams) + 255) & ~255u);
    if (!screenParamsCb_) {
        Utils::Alert("TextCommon: failed to create screen params buffer");
        return;
    }
    screenParamsCb_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedScreenParams_));

    LoadFont(DEFAULT_FONT);

#ifdef _DEBUG
    RegisterDebug("font_section", [this]() {
        if (ImGui::CollapsingHeader("Font")) {
            ImGui::InputText("Font Path", fontPathBuf_, sizeof(fontPathBuf_));
            ImGui::SameLine();
            if (ImGui::Button("...##BrowseFont")) {
                std::string selectedFile = Utils::OpenFileDialog("TTF");
                if (!selectedFile.empty()) {
                    strncpy_s(fontPathBuf_, selectedFile.c_str(), sizeof(fontPathBuf_) - 1);
                }
            }

            if (ImGui::Button("Load")) {
                const bool ok = LoadFont(fontPathBuf_);
                Log::Send(
                    ok ? Log::Level::INFO : Log::Level::WARNING,
                    ok ? std::string("TextCommon: loaded: ") + fontPathBuf_
                       : std::string("TextCommon: failed: ") + fontPathBuf_);
            }
            ImGui::TextDisabled(fontReady_ ? "Ready" : "Not loaded");
        }
        if (fontReady_ && ImGui::CollapsingHeader("Stats")) {
            ImGui::Text("Atlas glyphs : %zu",  glyphs_.size());
            ImGui::Text("Pending      : %u",   pendingGlyphs_);
        }

        if (ImGui::CollapsingHeader("TextBoxes")) {
            ImGui::Text("Count: %zu", texts_.size() + entries_.size());
            ImGui::SameLine();
            if (ImGui::SmallButton("+ Add")) {
                int n = static_cast<int>(entries_.size());
                std::string name = "TextBox_" + std::to_string(n);
                while (entries_.contains(name)) {
                    name = "TextBox_" + std::to_string(++n);
                }
                entries_[name] = TextData{};
                entryOrder_.insert(entryOrder_.begin(), name);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Save")) {
                SaveToJson();
            }
            ImGui::Separator();

            std::pair<std::string, std::string> pendingRename;
            for (size_t i = 0; i < entryOrder_.size(); ++i) {
                const std::string& name = entryOrder_[i];
                auto it = entries_.find(name);
                if (it == entries_.end()) continue;
                TextData& data = it->second;

                ImGui::PushID(static_cast<int>(i));

                ImGui::Checkbox("##vis", &data.visible);
                ImGui::SameLine();

                const std::string preview = data.text.size() > 20
                    ? data.text.substr(0, 20) + "..."
                    : data.text;
                const bool open = ImGui::TreeNode("##node", "[%s] \"%s\"",
                                                  name.c_str(), preview.c_str());

                if (open) {
                    static char nameBuf[64];
                    strncpy_s(nameBuf, name.c_str(), sizeof(nameBuf) - 1);
                    ImGui::SetNextItemWidth(160.f);
                    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf),
                                         ImGuiInputTextFlags_EnterReturnsTrue)) {
                        const std::string newName(nameBuf);
                        if (!newName.empty() && newName != name && !entries_.contains(newName)) {
                            pendingRename = { name, newName };
                        }
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Enter to rename");
                    }

                    static char textBuf[256];
                    strncpy_s(textBuf, data.text.c_str(), sizeof(textBuf) - 1);
                    if (ImGui::InputText("Text", textBuf, sizeof(textBuf))) {
                        data.text = textBuf;
                    }

                    float xy[2] = { data.position.x, data.position.y };
                    if (ImGui::DragFloat2("Position", xy, 1.f)) {
                        data.position = { xy[0], xy[1] };
                    }

                    if (ImGui::DragFloat("Font Size", &data.fontSize, 0.5f, 4.f, 256.f)) {}

                    float c[4] = { data.color.x, data.color.y, data.color.z, data.color.w };
                    if (ImGui::ColorEdit4("Color", c)) {
                        data.color = { c[0], c[1], c[2], c[3] };
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }

            // リネームをループ外で適用
            if (!pendingRename.first.empty()) {
                TextData moved = std::move(entries_[pendingRename.first]);
                entries_.erase(pendingRename.first);
                entries_[pendingRename.second] = std::move(moved);
                auto orderIt = std::ranges::find(entryOrder_, pendingRename.first);
                if (orderIt != entryOrder_.end()) *orderIt = pendingRename.second;
            }

            if (!texts_.empty()) {
                ImGui::Separator();
                ImGui::TextDisabled("Game Code");
                for (auto* text : texts_) {
                    ImGui::PushID(text);

                    bool vis = text->IsVisible();
                    if (ImGui::Checkbox("##vis", &vis)) { text->SetVisible(vis); }
                    ImGui::SameLine();

                    const std::string& content = text->GetText();
                    const std::string preview = content.size() > 20
                        ? content.substr(0, 20) + "..."
                        : content;
                    const bool open = ImGui::TreeNode("##node", "\"%s\"", preview.c_str());

                    if (open) {
                        static char textBuf[256];
                        strncpy_s(textBuf, content.c_str(), sizeof(textBuf) - 1);
                        if (ImGui::InputText("Text", textBuf, sizeof(textBuf))) {
                            text->SetText(textBuf);
                        }

                        const Vector2 pos = text->GetPosition();
                        float xy[2] = { pos.x, pos.y };
                        if (ImGui::DragFloat2("Position", xy, 1.f)) {
                            text->SetPosition(xy[0], xy[1]);
                        }

                        float size = text->GetFontSize();
                        if (ImGui::DragFloat("Font Size", &size, 0.5f, 4.f, 256.f)) {
                            text->SetFontSize(size);
                        }

                        const Vector4 col = text->GetColor();
                        float c[4] = { col.x, col.y, col.z, col.w };
                        if (ImGui::ColorEdit4("Color", c)) {
                            text->SetColor({ c[0], c[1], c[2], c[3] });
                        }

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }
            }
        }

    });
#endif
}

void TextCommon::Initialize(const GESTD::ReferencePtr<DirectXAdapter>&, const GESTD::ReferencePtr<DebugUI>&){
    Utils::Alert("TextCommon: use Initialize(adapter, debugUi, texture, srv)");
}

TextCommon::~TextCommon() {
    delete static_cast<stbtt_fontinfo*>(fontInfo_);
}

bool TextCommon::LoadFont(const std::string& _path) {
    std::ifstream file(_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Log::Send(Log::Level::WARNING,
            "TextCommon::LoadFont: cannot open file: " + _path);
        return false;
    }
    const auto fileSize = static_cast<size_t>(file.tellg());
    fontData_.resize(fileSize);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(fontData_.data()), static_cast<std::streamsize>(fileSize));

    delete static_cast<stbtt_fontinfo*>(fontInfo_);
    fontInfo_ = new stbtt_fontinfo{};
    auto* info = static_cast<stbtt_fontinfo*>(fontInfo_);

    if (!stbtt_InitFont(info, fontData_.data(), 0)) {
        Log::Send(Log::Level::WARNING,
            "TextCommon::LoadFont: stbtt_InitFont failed: " + _path);
        return false;
    }

    const float scale = stbtt_ScaleForPixelHeight(info, kAtlasFontPx);
    fontScale_ = scale;

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);
    fontAscent_ = static_cast<float>(ascent) * scale;

    std::vector<uint8_t> atlas(kAtlasW * kAtlasH, 0);
    glyphs_.clear();

    int curX = 0, curY = 0, rowH = 0;

    for (int cp = 32; cp <= 126; ++cp) {
        const int glyph = stbtt_FindGlyphIndex(info, cp);
        if (glyph == 0) continue;

        int w, h, xoff, yoff;
        uint8_t* sdf = stbtt_GetGlyphSDF(
            info, scale, glyph,
            kSdfPadding, kOnEdgeValue, kPixelDistScale,
            &w, &h, &xoff, &yoff);

        if (!sdf) {
            int advWidth;
            stbtt_GetGlyphHMetrics(info, glyph, &advWidth, nullptr);
            glyphs_[static_cast<uint32_t>(cp)] = { 0, 0, 0, 0, 0, 0,
                static_cast<float>(advWidth) * scale };
            continue;
        }

        if (curX + w > kAtlasW) { curX = 0; curY += rowH; rowH = 0; }
        if (curY + h > kAtlasH) { stbtt_FreeSDF(sdf, nullptr); break; }

        for (int row = 0; row < h; ++row) {
            std::memcpy(
                atlas.data() + (curY + row) * kAtlasW + curX,
                sdf + row * w,
                static_cast<size_t>(w));
        }

        int advWidth;
        stbtt_GetGlyphHMetrics(info, glyph, &advWidth, nullptr);

        glyphs_[static_cast<uint32_t>(cp)] = {
            curX, curY, w, h, xoff, yoff,
            static_cast<float>(advWidth) * scale
        };

        curX += w;
        rowH = std::max(rowH, h);
        stbtt_FreeSDF(sdf, nullptr);
    }

    atlasKey_ = "__text_atlas__" + _path;
    if (!texture_->LoadFromRawPixels(
            atlasKey_,
            atlas.data(), kAtlasW, kAtlasH,
            DXGI_FORMAT_R8_UNORM))
    {
        Log::Send(Log::Level::WARNING,
            "TextCommon::LoadFont: failed to upload atlas for: " + _path);
        return false;
    }

    fontReady_ = true;
    Log::Send(Log::Level::INFO, "TextCommon::LoadFont: loaded: " + _path);
    return true;
}

void TextCommon::RegisterText(Text* _text) {
    if (!_text) return;
    std::lock_guard lock(mutex_);
    texts_.push_back(_text);
}

void TextCommon::UnregisterText(Text* _text) {
    if (!_text) return;
    std::lock_guard lock(mutex_);
    std::erase(texts_, _text);
}

void TextCommon::SubmitEntry(const std::string& _text, float _x, float _y, float _fontSize, float _r, float _g, float _b, float _a) {
    if (!fontReady_ || !mappedInstances_) return;

    const float sizeScale = _fontSize / kAtlasFontPx;
    const float baseline  = _y + fontAscent_ * sizeScale;
    float penX = _x;

    for (const unsigned char ch : _text) {
        if (pendingGlyphs_ >= kMaxGlyphs) break;

        auto it = glyphs_.find(static_cast<uint32_t>(ch));
        if (it == glyphs_.end()) continue;

        const GlyphMetrics& g = it->second;

        if (g.atlasW == 0) {
            penX += g.advance * sizeScale;
            continue;
        }

        GlyphInstance& inst = mappedInstances_[pendingGlyphs_++];
        inst.posX  = penX  + static_cast<float>(g.bearingX) * sizeScale;
        inst.posY  = baseline + static_cast<float>(g.bearingY) * sizeScale;
        inst.sizeX = static_cast<float>(g.atlasW) * sizeScale;
        inst.sizeY = static_cast<float>(g.atlasH) * sizeScale;
        inst.uvX   = static_cast<float>(g.atlasX) / static_cast<float>(kAtlasW);
        inst.uvY   = static_cast<float>(g.atlasY) / static_cast<float>(kAtlasH);
        inst.uvW   = static_cast<float>(g.atlasW) / static_cast<float>(kAtlasW);
        inst.uvH   = static_cast<float>(g.atlasH) / static_cast<float>(kAtlasH);
        inst.r = _r; inst.g = _g; inst.b = _b; inst.a = _a;

        penX += g.advance * sizeScale;
    }
}

void TextCommon::Draw(Renderer* _renderer) {
    for (const auto& name : entryOrder_) {
        const auto it = entries_.find(name);
        if (it == entries_.end() || !it->second.visible || it->second.text.empty()) continue;
        const TextData& d = it->second;
        SubmitEntry(d.text, d.position.x, d.position.y, d.fontSize,
                    d.color.x, d.color.y, d.color.z, d.color.w);
    }

    const uint32_t count = pendingGlyphs_;
    pendingGlyphs_ = 0;

    if (count == 0 || !fontReady_ || !pipeline_) return;

    if (mappedScreenParams_) {
        mappedScreenParams_->screenW = static_cast<float>(adapter_->GetWidth());
        mappedScreenParams_->screenH = static_cast<float>(adapter_->GetHeight());
    }

    const D3D12_GPU_VIRTUAL_ADDRESS cbAddr =
        screenParamsCb_->Get()->GetGPUVirtualAddress();
    const D3D12_GPU_DESCRIPTOR_HANDLE glyphHandle = glyphGpuHandle_;
    const D3D12_GPU_DESCRIPTOR_HANDLE atlasHandle =
        texture_->GetGPUHandle(atlasKey_);

    _renderer->Register([this, count, cbAddr, glyphHandle, atlasHandle]() {
        pipeline_->DrawCall();

        const auto cmd = adapter_->GetCommandList();

        cmd->SetGraphicsRootConstantBufferView(0, cbAddr);
        cmd->SetGraphicsRootDescriptorTable(1, glyphHandle);
        cmd->SetGraphicsRootDescriptorTable(2, atlasHandle);

        cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmd->DrawInstanced(6, count, 0, 0);
    });
}

void TextCommon::LoadFromJson() {
    std::ifstream file(kJsonFile.data());
    if (!file.is_open()) return;

    using json = nlohmann::json;
    json root;
    file >> root;

    const auto it = root.find("textboxes");
    if (it == root.end() || !it->is_array()) return;

    for (const auto& entry : *it) {
        const std::string name = entry.value("name", "");
        if (name.empty()) continue;

        TextData data;
        data.text        = entry.value("text",     "");
        data.position.x  = entry.value("x",        0.f);
        data.position.y  = entry.value("y",        0.f);
        data.fontSize    = entry.value("fontSize", 32.f);
        data.visible     = entry.value("visible",  true);

        if (entry.contains("color") && entry["color"].is_array() && entry["color"].size() == 4) {
            data.color = {
                entry["color"][0].get<float>(),
                entry["color"][1].get<float>(),
                entry["color"][2].get<float>(),
                entry["color"][3].get<float>()
            };
        }

        entries_[name] = std::move(data);
        entryOrder_.push_back(name);
    }

    Log::Send(Log::Level::INFO, "TextCommon: loaded texts.json");
}

void TextCommon::SaveToJson() {
    using json = nlohmann::json;

    json root;
    root["textboxes"] = json::array();

    for (const auto& name : entryOrder_) {
        const auto it = entries_.find(name);
        if (it == entries_.end()) continue;
        const TextData& d = it->second;

        json entry;
        entry["name"]     = name;
        entry["text"]     = d.text;
        entry["x"]        = d.position.x;
        entry["y"]        = d.position.y;
        entry["fontSize"] = d.fontSize;
        entry["color"]    = { d.color.x, d.color.y, d.color.z, d.color.w };
        entry["visible"]  = d.visible;

        root["textboxes"].push_back(std::move(entry));
    }

    std::filesystem::create_directories(kJsonDir.data());
    std::ofstream file(kJsonFile.data());
    if (!file.is_open()) {
        Log::Send(Log::Level::ERR, "TextCommon: failed to save texts.json");
        return;
    }
    file << root.dump(4) << '\n';
    Log::Send(Log::Level::INFO, "TextCommon: saved texts.json");
}

void TextCommon::SetupPipeline() {
    pipeline_ = std::make_unique<PipelineStateObject>(adapter_);

    D3D12_DESCRIPTOR_RANGE glyphRange{
        .RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors     = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace      = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

    D3D12_DESCRIPTOR_RANGE atlasRange{
        .RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors     = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace      = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

    pipeline_->SetRootSignature(
        RootSignature()
            .AddParameter({
                .ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor       = { .ShaderRegister = 0 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
            })
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = {
                    .NumDescriptorRanges = 1,
                    .pDescriptorRanges   = &glyphRange
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
            })
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = {
                    .NumDescriptorRanges = 1,
                    .pDescriptorRanges   = &atlasRange
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            .SetSampler({
                .Filter         = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                .AddressU       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                .AddressV       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                .AddressW       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
                .MaxLOD         = D3D12_FLOAT32_MAX,
                .ShaderRegister = 0,
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
    )
    .SetBlend(BlendMode::ALPHA)
    .SetShader(std::make_unique<Shader>(L"Text"))
    .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
    .Create();
}
