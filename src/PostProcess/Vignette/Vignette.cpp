#include "Vignette.hpp"

#include "imgui.h"
#include "Log.hpp"
#include <filesystem>
#include <fstream>

#include "Math/MathUtils.hpp"
#include "vendor/json/json.hpp"

using json = nlohmann::json;

void Vignette::Initialize() {
    D3D12_DESCRIPTOR_RANGE range{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

    //Create PSO
    pso_ = std::make_unique<PipelineStateObject>(adapter_);
    pso_->SetRootSignature(
        RootSignature().AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
            .DescriptorTable = {
                .NumDescriptorRanges = 1,
                .pDescriptorRanges = &range
            },
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
        })
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
            .Descriptor = {
                .ShaderRegister = 0,
                .RegisterSpace = 0
            },
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
        })
        .SetSampler({
            .Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
            .MaxLOD = D3D12_FLOAT32_MAX,
            .ShaderRegister = 0,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
        })
    )
    .SetBlend(BlendMode::ALPHA)
    .SetShader(std::make_unique<Shader>(L"CpyImg", L"Vignette"))
    .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
    .Create();

    mr_ = adapter_->CreateBufferResource(sizeof(Material));
    mr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&material_));

    material_->color = {1.f, 1.f, 1.f, 1.f};
    material_->intensity = 16.f;
    material_->scale = 0.8f;
}

void Vignette::Debug() {
    ImGui::ColorEdit3("Color", &material_->color.x);
    ImGui::DragFloat("FrameScale", &material_->scale, 0.01f, 0.f, 1.f);
    ImGui::DragFloat("Density", &material_->color.w, 0.001f, 0.f, 1.f);
    ImGui::DragFloat("Intensity", &material_->intensity, 0.1f, 5.f, 100.f);
}

void Vignette::Modifier() {
    adapter_->GetCommandList()->SetGraphicsRootConstantBufferView(1, mr_->Get()->GetGPUVirtualAddress());
}

void Vignette::LoadPreset(const std::string& _presetName) {
    std::string path = "./Assets/Data/PostEffect/Vignette/" + _presetName + ".json";

    // ファイル不在時はデフォルト値
    if (!std::filesystem::exists(path)) {
        keyframes_.clear();
        keyframes_["Start"] = {0.0f, 1.0f, Vector4(0.f, 0.f, 0.f, 1.f)};
        keyframes_["End"] = {0.5f, 1.2f, Vector4(0.05f, 0.05f, 0.05f, 1.f)};
        keyframeOrder_ = {"Start", "End"};
        return;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        Log::Send(Log::Level::WARNING, std::format("Failed to open preset file: {}", path));
        return;
    }

    json paramJson;
    file >> paramJson;
    file.close();

    // 任意名キーフレーム読み込み
    keyframes_.clear();
    for (auto& [name, data] : paramJson.items()) {
        if (name == "keyframes") continue;
        KeyframeData kf;
        kf.intensity = data["intensity"];
        kf.scale = data["scale"];
        kf.color = Vector4(data["color"][0], data["color"][1], data["color"][2]);
        keyframes_[name] = kf;
    }

    if (paramJson.contains("keyframes")) {
        keyframeOrder_ = paramJson["keyframes"].get<std::vector<std::string>>();
    }

    // キーフレーム検証
    for (const auto& name : keyframeOrder_) {
        if (keyframes_.find(name) == keyframes_.end()) {
            Log::Send(Log::Level::WARNING, std::format("Keyframe '{}' not _found, using defaults", name));
            keyframes_.clear();
            keyframes_["Start"] = {0.0f, 1.0f, Vector4(0, 0, 0, 1)};
            keyframes_["End"] = {0.5f, 1.2f, Vector4(0.05f, 0.05f, 0.05f, 1.f)};
            keyframeOrder_ = {"Start", "End"};
            break;
        }
    }
}

void Vignette::UpdateAnimation(float _t) {
    // Q43: 単一キーフレーム対応
    if (keyframeOrder_.size() == 1) {
        const auto& kf = keyframes_[keyframeOrder_[0]];
        material_->intensity = kf.intensity;
        material_->scale = kf.scale;
        material_->color = Vector4(kf.color.x, kf.color.y, kf.color.z, material_->color.w);
        return;
    }

    if (keyframeOrder_.size() < 2) return;

    // セグメント計算
    float segmentCount = static_cast<float>(keyframeOrder_.size() - 1);
    float segmentProgress = _t * segmentCount;
    int currentSegment = static_cast<int>(segmentProgress);
    float segmentT = segmentProgress - currentSegment;

    if (currentSegment >= static_cast<int>(segmentCount)) {
        currentSegment = static_cast<int>(segmentCount) - 1;
        segmentT = 1.0f;
    }

    const auto& startKf = keyframes_[keyframeOrder_[currentSegment]];
    const auto& endKf = keyframes_[keyframeOrder_[currentSegment + 1]];

    // Linear補間
    material_->intensity = std::lerp(startKf.intensity, endKf.intensity, segmentT);
    material_->scale = std::lerp(startKf.scale, endKf.scale, segmentT);
    Vector4 color = MathUtils::Lerp(startKf.color, endKf.color, segmentT);
    material_->color = Vector4(color.x, color.y, color.z, material_->color.w);
}

void Vignette::SavePreset(const std::string& _presetName) {
    std::string path = "./Assets/Data/PostEffect/Vignette/" + _presetName + ".json";

    // ディレクトリ作成
    std::filesystem::create_directories("./Assets/Data/PostEffect/Vignette");

    json paramJson = SaveParameters();

    std::ofstream file(path);
    if (!file.is_open()) {
        Log::Send(Log::Level::ERR, std::format("Failed to save preset '{}': {}", _presetName, path));
        return;
    }

    file << paramJson.dump(4);
    file.close();
}

nlohmann::json Vignette::SaveParameters() const {
    json j;
    for (const auto& [name, kf] : keyframes_) {
        j[name]["intensity"] = kf.intensity;
        j[name]["scale"] = kf.scale;
        j[name]["color"] = {kf.color.x, kf.color.y, kf.color.z};
    }
    j["keyframes"] = keyframeOrder_;
    return j;
}
