#include "LineCommon.hpp"

#include "src/DirectX/GraphicsPipeline/Object/BlendMode.hpp"
#include "src/DirectX/GraphicsPipeline/Object/InputLayout.hpp"
#include "src/DirectX/GraphicsPipeline/Object/RootSignature.hpp"
#include "src/DirectX/Shader/Shader.h"

void LineCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) {
    Setup(_adapter, _debugUi);
    
    // PipelineStateObjectの初期化
    pipeline_ = std::make_unique<PipelineStateObject>(_adapter);
    
    // PipelineStateObject作成 (Line用)
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
    )
    .SetInputLayout(InputLayout{}// InputLayout設定 (LINE用)
        .SetElement({"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
    )
    .SetBlend(BlendMode::NONE)
    .SetShader(
        // シェーダー設定 (LINE用)
        std::make_unique<Shader>(L"Line")
    )
    .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE)
    .Create();
}

void LineCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, SRVManager* _srv) {
    srv_ = _srv;
    Initialize(_adapter, _debugUi);
}