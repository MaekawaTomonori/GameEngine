#include "PipelineStateObject.hpp"

#include <utility>

#include "Log.hpp"
#include "Utils.hpp"

PipelineStateObject::PipelineStateObject(DirectXAdapter* _adapter) :adapter_(_adapter){}

PipelineStateObject& PipelineStateObject::SetRootSignature(const RootSignature& _rootSignature) {
    rootSignature_ = std::move(_rootSignature);
    rootSignature_.Create(adapter_->GetDevice());
    return *this;
}

PipelineStateObject& PipelineStateObject::SetInputLayout(const InputLayout& _inputLayout) {
    inputLayout_ = std::move(_inputLayout);
    return *this;
}

PipelineStateObject& PipelineStateObject::SetBlendDesc(const D3D12_BLEND_DESC& _blendDesc) {
    blendDesc_ = _blendDesc;
    return *this;
}

PipelineStateObject& PipelineStateObject::SetShader(std::unique_ptr<Shader> _shader) {
    shader_ = std::move(_shader);
    return *this;
}

PipelineStateObject& PipelineStateObject::SetTopologyType(const D3D12_PRIMITIVE_TOPOLOGY_TYPE _topologyType) {
    topologyType_ = _topologyType;
    return *this;
}

void PipelineStateObject::Create() {
    desc_.pRootSignature = rootSignature_.Get();
    desc_.InputLayout = inputLayout_.Get();
    desc_.BlendState = blendDesc_;
    desc_.VS = { shader_->GetVertexShader()->GetBufferPointer(), shader_->GetVertexShader()->GetBufferSize() };
    desc_.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    desc_.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    desc_.PS = { shader_->GetPixelShader()->GetBufferPointer(), shader_->GetPixelShader()->GetBufferSize() };
    desc_.DepthStencilState.DepthEnable = false;

    desc_.NumRenderTargets = 1;
    desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    desc_.PrimitiveTopologyType = topologyType_;
    desc_.SampleDesc.Count = 1;
    desc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    // Setting more options
    
    HRESULT hr = adapter_->GetDevice()->CreateGraphicsPipelineState(&desc_, IID_PPV_ARGS(&pso_));
    if (FAILED(hr)){
        Log::Send(Log::Level::ERR, "Failed to create pipeline state object");
        Utils::Alert("Failed to create pipeline state object");
        return;
    } 
    Log::Send(Log::Level::INFO, "Pipeline state object created successfully");
}

void PipelineStateObject::DrawCall() const {
    if (!adapter_ || !pso_ || !rootSignature_.Get()) {
        Log::Send(Log::Level::ERR, "PipelineStateObject is not properly initialized");
        return;
    }
    
    adapter_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
    adapter_->GetCommandList()->SetPipelineState(pso_.Get());
}
