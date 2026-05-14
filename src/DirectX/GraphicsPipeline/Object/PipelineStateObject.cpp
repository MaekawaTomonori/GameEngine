#include "PipelineStateObject.hpp"

#include <utility>

#include "Log.hpp"
#include "Utils.hpp"

PipelineStateObject::PipelineStateObject(GESTD::ReferencePtr<DirectXAdapter> _adapter) :adapter_(_adapter){}

PipelineStateObject& PipelineStateObject::SetRootSignature(const RootSignature& _rootSignature) {
    rootSignature_ = std::move(_rootSignature);
    rootSignature_.Create(adapter_->GetDevice());
    return *this;
}

PipelineStateObject& PipelineStateObject::SetInputLayout(const InputLayout& _inputLayout) {
    inputLayout_ = std::move(_inputLayout);
    return *this;
}

PipelineStateObject& PipelineStateObject::SetBlend(const D3D12_BLEND_DESC& _blendDesc) {
    blendDesc_ = _blendDesc;
    return *this;
}

PipelineStateObject& PipelineStateObject::SetBlend(const BlendMode _blendMode) {
    blendDesc_.RenderTarget[0].BlendEnable = true;
    blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    switch (_blendMode){
        case BlendMode::ALPHA:
            blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            // アルファチャンネルはデスティネーション値（1.0）を維持する
            // result.a = src.a * 0 + dst.a * 1 = dst.a
            // → renderTexture_ のアルファが透過スプライトで 0 になることを防ぐ
            blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
            break;
        case BlendMode::ADD:
            blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
            blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            break;
        case BlendMode::SUB:
            blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
            blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
            break;
        case BlendMode::MULTI:
            blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
            blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_DEST_COLOR;
            blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
            break;
        case BlendMode::SCREEN:
            blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
            blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
            break;
        case BlendMode::NONE:
            blendDesc_.RenderTarget[0].BlendEnable = false;
            blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    return *this;
}

PipelineStateObject& PipelineStateObject::SetRasterizer(const D3D12_RASTERIZER_DESC _rasterizer) {
    rasterizer_ = _rasterizer;
    return *this;
}

PipelineStateObject& PipelineStateObject::SetDepthStencil(const D3D12_DEPTH_STENCIL_DESC& _depthStencilDesc) {
    depthStencilDesc_ = _depthStencilDesc;
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

PipelineStateObject& PipelineStateObject::SetDSVFormat(DXGI_FORMAT _format) {
    desc_.DSVFormat = _format;
    return *this;
}

PipelineStateObject& PipelineStateObject::SetRTVFormat(DXGI_FORMAT _format, UINT _numTargets) {
    rtvFormat_ = _format;
    numRenderTargets_ = _numTargets;
    return *this;
}

void PipelineStateObject::Create() {
    desc_.pRootSignature = rootSignature_.Get();
    desc_.InputLayout = inputLayout_.Get();
    desc_.BlendState = blendDesc_;
    desc_.VS = { shader_->GetVertexShader()->GetBufferPointer(), shader_->GetVertexShader()->GetBufferSize() };
    desc_.RasterizerState = rasterizer_;
    desc_.PS = { shader_->GetPixelShader()->GetBufferPointer(), shader_->GetPixelShader()->GetBufferSize() };
    desc_.DepthStencilState = depthStencilDesc_;

    desc_.NumRenderTargets = numRenderTargets_;
    desc_.RTVFormats[0] = (numRenderTargets_ > 0) ? rtvFormat_ : DXGI_FORMAT_UNKNOWN;

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
