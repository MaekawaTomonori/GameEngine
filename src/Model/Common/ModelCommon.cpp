#include "ModelCommon.hpp"

#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/GraphicsPipeline/Object/BlendMode.hpp"
#include "src/DirectX/GraphicsPipeline/Object/InputLayout.hpp"
#include "src/DirectX/GraphicsPipeline/Object/RootSignature.hpp"
#include "src/DirectX/Shader/Shader.h"

void ModelCommon::Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi) {
	Setup(_adapter, _debugUi);
	
	// PipelineStateObjectの初期化
	pipeline_ = std::make_unique<PipelineStateObject>(_adapter);
	
	// RootSignature設定 (SKINNING_MODEL用)
	RootSignature rootSignature;
	D3D12_ROOT_PARAMETER rootParameter = {};
	
	// 1. Material CBV (Pixel Shader, register 0)
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter.Descriptor.ShaderRegister = 0;
	rootSignature.AddParameter(rootParameter);
	
	// 2. Transform CBV (Vertex Shader, register 0)
	rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter.Descriptor.ShaderRegister = 0;
	rootSignature.AddParameter(rootParameter);
	
	// 3. Texture SRV (Pixel Shader, Descriptor Table)
	D3D12_DESCRIPTOR_RANGE textureRange = {};
	textureRange.BaseShaderRegister = 0;
	textureRange.NumDescriptors = 1;
	textureRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textureRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter.DescriptorTable.pDescriptorRanges = &textureRange;
	rootParameter.DescriptorTable.NumDescriptorRanges = 1;
	rootSignature.AddParameter(rootParameter);
	
	// 4. DirectionalLight SRV (Pixel Shader, register 1)
	rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter.Descriptor.ShaderRegister = 1;
	rootSignature.AddParameter(rootParameter);
	
	// 5. Camera CBV (Pixel Shader, register 2)
	rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter.Descriptor.ShaderRegister = 2;
	rootSignature.AddParameter(rootParameter);
	
	// 6. PointLight SRV (Pixel Shader, register 3)
	rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter.Descriptor.ShaderRegister = 3;
	rootSignature.AddParameter(rootParameter);
	
	// 7. SpotLight SRV (Pixel Shader, register 4)
	rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter.Descriptor.ShaderRegister = 4;
	rootSignature.AddParameter(rootParameter);
	
	// 8. Light Count CBV (Pixel Shader, register 5)
	rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter.Descriptor.ShaderRegister = 5;
	rootSignature.AddParameter(rootParameter);
	
	// 9. Animation SRV (Vertex Shader, Descriptor Table)
	D3D12_DESCRIPTOR_RANGE animationRange = {};
	animationRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	animationRange.NumDescriptors = 1;
	animationRange.BaseShaderRegister = 0;
	animationRange.RegisterSpace = 0;
	animationRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter.DescriptorTable.NumDescriptorRanges = 1;
	rootParameter.DescriptorTable.pDescriptorRanges = &animationRange;
	rootSignature.AddParameter(rootParameter);
	
	// サンプラー設定
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootSignature.SetSampler(sampler);
	
	// InputLayout設定 (SKINNING_MODEL用)
	InputLayout inputLayout;
	
	// Slot 0: Position, TexCoord, Normal
	inputLayout.SetElement({"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
			   .SetElement({"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
			   .SetElement({"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
			   // Slot 1: Weight, Index
			   .SetElement({"WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
			   .SetElement({"INDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0});
	
	// シェーダー設定 (SKINNING_MODEL用)
	auto shader = std::make_unique<Shader>(L"Skinning");
	
	// PipelineStateObject作成
	pipeline_->SetRootSignature(rootSignature)
			  .SetInputLayout(inputLayout)
			  .SetBlend(BlendMode::ALPHA)
			  .SetShader(std::move(shader))
			  .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
			  .Create();
}

void ModelCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, ResourceRepository* _resource, SRVManager* _srv) {
    resource_ = _resource;
    srv_ = _srv;
    Initialize(_adapter, _debugUi);
}
