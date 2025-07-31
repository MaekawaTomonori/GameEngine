#include "ModelCommon.hpp"

#include "Log.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/GraphicsPipeline/Object/BlendMode.hpp"
#include "src/DirectX/GraphicsPipeline/Object/InputLayout.hpp"
#include "src/DirectX/GraphicsPipeline/Object/RootSignature.hpp"
#include "src/DirectX/Shader/Shader.h"

void ModelCommon::Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi) {
	Setup(_adapter, _debugUi);
	
	// PipelineStateObjectの初期化 (Skinning用)
	pipeline_ = std::make_unique<PipelineStateObject>(_adapter);
	
	// PipelineStateObjectの初期化 (Static用)
	staticPipeline_ = std::make_unique<PipelineStateObject>(_adapter);
	
	// Skinning Model用のパイプライン作成
	CreateSkinningPipeline();
	
	// Static Model用のパイプライン作成
	CreateStaticPipeline();
}

void ModelCommon::CreateSkinningPipeline() const {
	// RootSignature設定 (SKINNING_MODEL用)
    // for 3. Texture SRV (Pixel Shader, Descriptor Table)
	D3D12_DESCRIPTOR_RANGE textureRange = {};
	textureRange.BaseShaderRegister = 0;
	textureRange.NumDescriptors = 1;
	textureRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textureRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // for 9. Environment TextureCube
    D3D12_DESCRIPTOR_RANGE environmentRange = {};
	environmentRange.BaseShaderRegister = 5;
	environmentRange.NumDescriptors = 1;
	environmentRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    environmentRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // for 10. Animation SRV
    D3D12_DESCRIPTOR_RANGE animationRange = {};
	animationRange.BaseShaderRegister = 0;
	animationRange.NumDescriptors = 1;
	animationRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    animationRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// PipelineStateObject作成 (Skinning用)
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
        // 4. DirectionalLight SRV (Pixel Shader, register 1)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 1
        })
        // 5. Camera CBV (Pixel Shader, register 2)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 2
        })
        // 6. PointLight SRV (Pixel Shader, register 3)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 3
        })
        // 7. SpotLight SRV (Pixel Shader, register 4)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 4
        })
        // 8. Light Count CBV (Pixel Shader, register 5)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 5
        })
        // 9. Environment TextureCube SRV (Pixel Shader, Descriptor Table, register 5)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .DescriptorTable.pDescriptorRanges = &environmentRange,
            .DescriptorTable.NumDescriptorRanges = 1
        })
        // 10. Animation SRV (Vertex Shader, Descriptor Table)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX,
            .DescriptorTable.pDescriptorRanges = &animationRange,
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
	.SetInputLayout(InputLayout{}// InputLayout設定 (SKINNING_MODEL用 - WEIGHTとINDEX含む)
		.SetElement({"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .SetElement({"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .SetElement({"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        // Slot 1: Weight, Index
        .SetElement({"WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .SetElement({"INDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
	 )
	.SetBlend(BlendMode::ALPHA)
	.SetShader(
	    // シェーダー設定 (SKINNING_MODEL用)
	    std::make_unique<Shader>(L"Skinning")
    )
	.SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
	.Create();
}

void ModelCommon::CreateStaticPipeline() const {
	// RootSignature設定 (STATIC_MODEL用)
    // for 3. Texture SRV (Pixel Shader, Descriptor Table)
	D3D12_DESCRIPTOR_RANGE textureRange = {};
	textureRange.BaseShaderRegister = 0;
	textureRange.NumDescriptors = 1;
	textureRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textureRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // for 9. Light Count
    D3D12_DESCRIPTOR_RANGE staticEnvironmentRange = {};
	staticEnvironmentRange.BaseShaderRegister = 5;
	staticEnvironmentRange.NumDescriptors = 1;
	staticEnvironmentRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    staticEnvironmentRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// PipelineStateObject作成 (Static用)
	staticPipeline_->SetRootSignature(RootSignature{}
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
        // 4. DirectionalLight SRV (Pixel Shader, register 1)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 1
        })
        // 5. Camera CBV (Pixel Shader, register 2)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 2
        })
        // 6. PointLight SRV (Pixel Shader, register 3)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 3
        })
        // 7. SpotLight SRV (Pixel Shader, register 4)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 4
        })
        // 8. Light Count CBV (Pixel Shader, register 5)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .Descriptor.ShaderRegister = 5
        })
        // 9. Environment TextureCube SRV (Pixel Shader, Descriptor Table, register 5)
        .AddParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
            .DescriptorTable.pDescriptorRanges = &staticEnvironmentRange,
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
	.SetInputLayout(InputLayout{}// InputLayout設定 (STATIC_MODEL用 - WEIGHTとINDEXなし)
		.SetElement({"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .SetElement({"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .SetElement({"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
	 )
	.SetBlend(BlendMode::ALPHA)
	.SetShader(
	    // シェーダー設定 (STATIC_MODEL用)
	    std::make_unique<Shader>(L"Model")
    )
	.SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
	.Create();
	Log::Send(Log::Level::INFO, "Static model pipeline created successfully");
}

void ModelCommon::DrawSkinning() const {
	pipeline_->DrawCall();
}

void ModelCommon::DrawStatic() const {
	staticPipeline_->DrawCall();
}

void ModelCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, ResourceRepository* _resource, SRVManager* _srv) {
    resource_ = _resource;
    srv_ = _srv;
    Initialize(_adapter, _debugUi);
}
