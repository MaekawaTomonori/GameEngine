#include "LineCommon.hpp"

#include "src/DirectX/GraphicsPipeline/Object/BlendMode.hpp"
#include "src/DirectX/GraphicsPipeline/Object/InputLayout.hpp"
#include "src/DirectX/GraphicsPipeline/Object/RootSignature.hpp"
#include "src/DirectX/Shader/Shader.h"

void LineCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) {
    Setup(_adapter, _debugUi);
    
    // PipelineStateObjectの初期化
    pipeline_ = std::make_unique<PipelineStateObject>(_adapter);
    
    // RootSignature設定 (LINE用)
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
    
    // InputLayout設定 (LINE用)
    InputLayout inputLayout;
    inputLayout.SetElement({"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0});
    
    // シェーダー設定 (LINE用)
    auto shader = std::make_unique<Shader>(L"Line");
    
    // PipelineStateObject作成
    pipeline_->SetRootSignature(rootSignature)
              .SetInputLayout(inputLayout)
              .SetBlend(BlendMode::NONE)
              .SetShader(std::move(shader))
              .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE)
              .Create();
}

void LineCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, SRVManager* _srv) {
    srv_ = _srv;
    Initialize(_adapter, _debugUi);
}