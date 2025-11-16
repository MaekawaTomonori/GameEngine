#include "ComputePipeline.hpp"

#include "Log.hpp"
#include "Utils.hpp"
#include "src/DirectX/Shader/Shader.h"

ComputePipeline& ComputePipeline::SetRootSignature(const RootSignature& _rootSignature) {
    rootSignature_ = _rootSignature;
    return *this;
}

void ComputePipeline::Create(const std::string& _name) {
    Shader shader;
    shader_.Attach(shader.CompileCS(Utils::Convert(_name)));

    D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
    desc.pRootSignature = rootSignature_.Get();
    desc.CS = {
        shader_->GetBufferPointer(),
        shader_->GetBufferSize()
    };

    if (!adapter_.has_value()) {
        Log::Send(Log::Level::ERR, "DirectXAdapter is not set");
        Utils::Alert("DirectXAdapter is not set");
        throw std::runtime_error("DirectXAdapter is not set");
    }
    HRESULT hr = adapter_->get().GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&state_));
    if (FAILED(hr)) {
        Log::Send(Log::Level::ERR, "Failed to create compute pipeline state");
        Utils::Alert("Failed to create compute pipeline state");
        throw std::runtime_error("Failed to create compute pipeline state");
    }

}
