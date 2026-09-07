#include "Grayscale.hpp"

#include "imgui.h"

void Grayscale::Initialize() {
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
    .SetBlend(BlendMode::NONE)
    .SetShader(std::make_unique<Shader>(L"CpyImg", L"Grayscale"))
    .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
    .Create();

    mr_ = adapter_->CreateBufferResource(sizeof(Material));
    mr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&material_));

    material_->color = {1.f, 1.f, 1.f, 1.f};
}

void Grayscale::Debug() {
    ImGui::ColorEdit3("Color", &material_->color.x);
    if (ImGui::Button("Gray")) {
        material_->color = GRAY;
    }
    ImGui::SameLine();
    if (ImGui::Button("Sepia")){
        material_->color = SEPIA;
    }
}

void Grayscale::Modifier() {
    adapter_->GetCommandList()->SetGraphicsRootConstantBufferView(1, mr_->Get()->GetGPUVirtualAddress());
}

void Grayscale::LoadPreset(const std::string& _presetName) { (void)_presetName;}
void Grayscale::SavePreset(const std::string& _presetName) { (void)_presetName;}
nlohmann::json Grayscale::SaveParameters() const { return nlohmann::json();}
void Grayscale::UpdateAnimation(float _t) { (void)_t;}

nlohmann::json Grayscale::CaptureCurrentParameters() const {
    nlohmann::json j;
    j["color"] = {material_->color.x, material_->color.y, material_->color.z, material_->color.w};
    return j;
}

void Grayscale::ApplyParameters(const nlohmann::json& _params) {
    if (_params.contains("color")) {
        const auto& c = _params["color"];
        material_->color = Vector4(c[0], c[1], c[2], c[3]);
    }
}
