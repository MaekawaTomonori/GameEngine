#include "ModelCommon.hpp"

#include <ranges>

#include "Log.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/RootSignature/BlendMode.hpp"
#include "src/DirectX/RootSignature/InputLayout.hpp"
#include "src/DirectX/RootSignature/RootSignature.hpp"
#include "src/DirectX/Shader/Shader.h"

void ModelCommon::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi) {
	Setup(_adapter, _debugUi, "Model");
	debugUI_->RegisterMenuButton("Model");

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
	D3D12_DESCRIPTOR_RANGE textureRange{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

    D3D12_DESCRIPTOR_RANGE environmentRange{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 5,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

    D3D12_DESCRIPTOR_RANGE animationRange{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

    D3D12_DESCRIPTOR_RANGE shadowRange{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 6,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

	// PipelineStateObject作成 (Skinning用)
	pipeline_->SetRootSignature(
        RootSignature()
            // root 0: Material CBV (b0, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = { .ShaderRegister = 0 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 1: Transform CBV (b0, VS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = { .ShaderRegister = 0 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
            })
            // root 2: Texture SRV (t0, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = { .NumDescriptorRanges = 1, .pDescriptorRanges = &textureRange },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 3: DirectionalLight SRV (t1, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = { .ShaderRegister = 1 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 4: Camera CBV (b2, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = { .ShaderRegister = 2 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 5: PointLight SRV (t3, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = { .ShaderRegister = 3 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 6: SpotLight SRV (t4, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = { .ShaderRegister = 4 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 7: LightCount CBV (b5, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = { .ShaderRegister = 5 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 8: Environment TextureCube SRV (t5, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = { .NumDescriptorRanges = 1, .pDescriptorRanges = &environmentRange },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 9: Shadow Cube SRV (t6, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = { .NumDescriptorRanges = 1, .pDescriptorRanges = &shadowRange },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 10: Shadow Data CBV (b6, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = { .ShaderRegister = 6 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 11: Animation SRV (t0, VS) — 旧 root 9
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = { .NumDescriptorRanges = 1, .pDescriptorRanges = &animationRange },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
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
        .SetSampler({
            .Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            .AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            .AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
            .MaxLOD = D3D12_FLOAT32_MAX,
            .ShaderRegister = 1,
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
	.SetDepthStencil({
	    .DepthEnable = true,
	    .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
	    .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL
	})
	.SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT)
	.SetShader(
	    // シェーダー設定 (SKINNING_MODEL用)
	    std::make_unique<Shader>(L"Skinning")
    )
	.SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
	.Create();
}

void ModelCommon::CreateStaticPipeline() const {
	D3D12_DESCRIPTOR_RANGE textureRange{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

    D3D12_DESCRIPTOR_RANGE staticEnvironmentRange{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 5,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

    D3D12_DESCRIPTOR_RANGE shadowRange{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 6,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

	// PipelineStateObject作成 (Static用)
	staticPipeline_->SetRootSignature(
        RootSignature()
            // root 0: Material CBV (b0, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = { .ShaderRegister = 0 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 1: Transform CBV (b0, VS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = { .ShaderRegister = 0 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
            })
            // root 2: Texture SRV (t0, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = { .NumDescriptorRanges = 1, .pDescriptorRanges = &textureRange },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 3: DirectionalLight SRV (t1, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = { .ShaderRegister = 1 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 4: Camera CBV (b2, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = { .ShaderRegister = 2 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 5: PointLight SRV (t3, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = { .ShaderRegister = 3 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 6: SpotLight SRV (t4, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = { .ShaderRegister = 4 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 7: LightCount CBV (b5, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = { .ShaderRegister = 5 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 8: Environment TextureCube SRV (t5, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = { .NumDescriptorRanges = 1, .pDescriptorRanges = &staticEnvironmentRange },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 9: Shadow Cube SRV (t6, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = { .NumDescriptorRanges = 1, .pDescriptorRanges = &shadowRange },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // root 10: Shadow Data CBV (b6, PS)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = { .ShaderRegister = 6 },
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
        .SetSampler({
            .Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            .AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            .AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
            .MaxLOD = D3D12_FLOAT32_MAX,
            .ShaderRegister = 1,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
        })
    )
	.SetInputLayout(InputLayout{}// InputLayout設定 (STATIC_MODEL用 - WEIGHTとINDEXなし)
		.SetElement({"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .SetElement({"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .SetElement({"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
	 )
	.SetBlend(BlendMode::ALPHA)
	.SetDepthStencil({
	    .DepthEnable = true,
	    .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
	    .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL
	})
	.SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT)
	.SetShader(
	    // シェーダー設定 (STATIC_MODEL用)
	    std::make_unique<Shader>(L"Model")
    )
	.SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
	.Create();
	Log::Send(Log::Level::INFO, "Static model pipeline created successfully");
}

void ModelCommon::RegisterStaticDraw(const std::function<void()>& _command, bool _isApplyPostEffect) {
    std::lock_guard<std::mutex> lock(mutex_);
    staticDrawCommands_.push_back({ _command, _isApplyPostEffect });
}

void ModelCommon::RegisterSkinningDraw(const std::function<void()>& _command, bool _isApplyPostEffect) {
    std::lock_guard<std::mutex> lock(mutex_);
    skinningDrawCommands_.push_back({ _command, _isApplyPostEffect });
}

void ModelCommon::RegisterShadowDraw(const std::string& _id, const std::function<void()>& _func) {
    std::lock_guard<std::mutex> lock(mutex_);
    shadowCommands_[_id] = _func;
}

void ModelCommon::UnregisterShadowDraw(const std::string& _id) {
    std::lock_guard<std::mutex> lock(mutex_);
    shadowCommands_.erase(_id);
}

void ModelCommon::ExecuteShadowDraw() const {
    for (const auto& func : shadowCommands_ | std::views::values) {
        func();
    }
}

void ModelCommon::SetShadowBinding(uint32_t _srvIndex, D3D12_GPU_VIRTUAL_ADDRESS _cbvAddress) {
    shadowSrvIndex_ = _srvIndex;
    shadowCbvAddress_ = _cbvAddress;
}

void ModelCommon::Draw(Renderer* _renderer) {
    std::vector<std::function<void()>> staticPostEffectTasks;
    std::vector<std::function<void()>> staticNoPostEffectTasks;
    std::vector<std::function<void()>> skinningPostEffectTasks;
    std::vector<std::function<void()>> skinningNoPostEffectTasks;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        for (const auto& command : staticDrawCommands_) {
            if (command.applyPostEffects) {
                staticPostEffectTasks.push_back(command.func);
            } else {
                staticNoPostEffectTasks.push_back(command.func);
            }
        }
        staticDrawCommands_.clear();

        for (const auto& command : skinningDrawCommands_) {
            if (command.applyPostEffects) {
                skinningPostEffectTasks.push_back(command.func);
            } else {
                skinningNoPostEffectTasks.push_back(command.func);
            }
        }
        skinningDrawCommands_.clear();
    }

    auto bindShadow = [this]() {
        if (shadowSrvIndex_ != UINT_MAX && srv_) {
            srv_->SetGraphicsRootDescriptorTable(9, shadowSrvIndex_);
            adapter_->GetCommandList()->SetGraphicsRootConstantBufferView(10, shadowCbvAddress_);
        }
    };

    if (!staticNoPostEffectTasks.empty()) {
        _renderer->Register([this, staticNoPostEffectTasks, bindShadow]() {
            if (staticPipeline_) {
                staticPipeline_->DrawCall();
            }
            bindShadow();
            for (auto& task : staticNoPostEffectTasks) {
                task();
            }
        });
    }

    if (!staticPostEffectTasks.empty()) {
        _renderer->Register([this, staticPostEffectTasks, bindShadow]() {
            if (staticPipeline_) {
                staticPipeline_->DrawCall();
            }
            bindShadow();
            for (auto& task : staticPostEffectTasks) {
                task();
            }
        }, true);
    }

    if (!skinningNoPostEffectTasks.empty()) {
        _renderer->Register([this, skinningNoPostEffectTasks, bindShadow]() {
            if (pipeline_) {
                pipeline_->DrawCall();
            }
            bindShadow();
            for (auto& task : skinningNoPostEffectTasks) {
                task();
            }
        });
    }

    if (!skinningPostEffectTasks.empty()) {
        _renderer->Register([this, skinningPostEffectTasks, bindShadow]() {
            if (pipeline_) {
                pipeline_->DrawCall();
            }
            bindShadow();
            for (auto& task : skinningPostEffectTasks) {
                task();
            }
        }, true);
    }
}

void ModelCommon::DrawSkinning() const {
	pipeline_->DrawCall();
}

void ModelCommon::DrawStatic() const {
	staticPipeline_->DrawCall();
}

void ModelCommon::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi, GESTD::ReferencePtr<ResourceRepository> _resource, SRVManager* _srv) {
    resource_ = _resource;
    srv_ = _srv;
    Initialize(_adapter, _debugUi);
}
