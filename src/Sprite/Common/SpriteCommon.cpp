#include "SpriteCommon.hpp"

#include "src/DirectX/GraphicsPipeline/Object/BlendMode.hpp"
#include "src/DirectX/GraphicsPipeline/Object/InputLayout.hpp"
#include "src/DirectX/GraphicsPipeline/Object/RootSignature.hpp"
#include "src/DirectX/Shader/Shader.h"

void SpriteCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) {
    Setup(_adapter, _debugUi);
    
    // PipelineStateObjectの初期化
    pipeline_ = std::make_unique<PipelineStateObject>(_adapter);
    
    // Descriptor Range for Texture
    D3D12_DESCRIPTOR_RANGE textureRange = {};
    textureRange.BaseShaderRegister = 0;
    textureRange.NumDescriptors = 1;
    textureRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    textureRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    
    // PipelineStateObject作成 (Sprite用)
    pipeline_->SetRootSignature(RootSignature{}
        // 1. Material CBV (Pixel Shader, register 0)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 0
        })
        // 2. Transform CBV (Vertex Shader, register 0)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX,
            .Descriptor.ShaderRegister = 0
        })
        // 3. Texture SRV (Pixel Shader, Descriptor Table)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .DescriptorTable.pDescriptorRanges = &textureRange,
            .DescriptorTable.NumDescriptorRanges = 1
        })
        // サンプラー設定
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
    .SetInputLayout(InputLayout{}
        .SetElement({"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .SetElement({"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
    )
    .SetBlend(BlendMode::ALPHA)
    .SetShader(std::make_unique<Shader>(L"Sprite"))
    .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
    .Create();
}

