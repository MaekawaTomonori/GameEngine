#include "ModelCommon.hpp"

#include "Log.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/RootSignature/BlendMode.hpp"
#include "src/DirectX/RootSignature/InputLayout.hpp"
#include "src/DirectX/RootSignature/RootSignature.hpp"
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

	// PipelineStateObject作成 (Skinning用)
	pipeline_->SetRootSignature(
        RootSignature()
            // 1. Material CBV (Pixel Shader, register 0)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = {
                    .ShaderRegister = 0
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 2. Transform CBV (Vertex Shader, register 0)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = {
                    .ShaderRegister = 0
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
            })
            // 3. Texture SRV (Pixel Shader, Descriptor Table)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = {
                    .NumDescriptorRanges = 1,
                    .pDescriptorRanges = &textureRange
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 4. DirectionalLight SRV (Pixel Shader, register 1)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = {
                    .ShaderRegister = 1
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 5. Camera CBV (Pixel Shader, register 2)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = {
                    .ShaderRegister = 2
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 6. PointLight SRV (Pixel Shader, register 3)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = {
                    .ShaderRegister = 3
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 7. SpotLight SRV (Pixel Shader, register 4)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = {
                    .ShaderRegister = 4
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 8. Light Count CBV (Pixel Shader, register 5)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = {
                    .ShaderRegister = 5
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 9. Environment TextureCube SRV (Pixel Shader, Descriptor Table, register 5)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = {
                    .NumDescriptorRanges = 1,
                    .pDescriptorRanges = &environmentRange
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 10. Animation SRV (Vertex Shader, Descriptor Table)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = {
                    .NumDescriptorRanges = 1,
                    .pDescriptorRanges = &animationRange
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
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

	// PipelineStateObject作成 (Static用)
	staticPipeline_->SetRootSignature(
        RootSignature()
            // 1. Material CBV (Pixel Shader, register 0)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = {
                    .ShaderRegister = 0
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 2. Transform CBV (Vertex Shader, register 0)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = {
                    .ShaderRegister = 0
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
            })
            // 3. Texture SRV (Pixel Shader, Descriptor Table)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = {
                    .NumDescriptorRanges = 1,
                    .pDescriptorRanges = &textureRange
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 4. DirectionalLight SRV (Pixel Shader, register 1)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = {
                    .ShaderRegister = 1
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 5. Camera CBV (Pixel Shader, register 2)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = {
                    .ShaderRegister = 2
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 6. PointLight SRV (Pixel Shader, register 3)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = {
                    .ShaderRegister = 3
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 7. SpotLight SRV (Pixel Shader, register 4)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                .Descriptor = {
                    .ShaderRegister = 4
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 8. Light Count CBV (Pixel Shader, register 5)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = {
                    .ShaderRegister = 5
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 9. Environment TextureCube SRV (Pixel Shader, Descriptor Table, register 5)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = {
                    .NumDescriptorRanges = 1,
                    .pDescriptorRanges = &staticEnvironmentRange
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
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

    if (!staticNoPostEffectTasks.empty()) {
        _renderer->Register([this, staticNoPostEffectTasks]() {
            if (staticPipeline_) {
                staticPipeline_->DrawCall();
            }
            for (auto& task : staticNoPostEffectTasks) {
                task();
            }
        });
    }

    if (!staticPostEffectTasks.empty()) {
        _renderer->Register([this, staticPostEffectTasks]() {
            if (staticPipeline_) {
                staticPipeline_->DrawCall();
            }
            for (auto& task : staticPostEffectTasks) {
                task();
            }
        }, true);
    }

    if (!skinningNoPostEffectTasks.empty()) {
        _renderer->Register([this, skinningNoPostEffectTasks]() {
            if (pipeline_) {
                pipeline_->DrawCall();
            }
            for (auto& task : skinningNoPostEffectTasks) {
                task();
            }
        });
    }

    if (!skinningPostEffectTasks.empty()) {
        _renderer->Register([this, skinningPostEffectTasks]() {
            if (pipeline_) {
                pipeline_->DrawCall();
            }
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

void ModelCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, ResourceRepository* _resource, SRVManager* _srv) {
    resource_ = _resource;
    srv_ = _srv;
    Initialize(_adapter, _debugUi);
}
