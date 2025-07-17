#include "PipelineStateObject.hpp"

#include <utility>

#include "Log.hpp"
#include "Utils.hpp"

PipelineStateObject::PipelineStateObject(DirectXAdapter* _adapter) :adapter_(_adapter){
}

PipelineStateObject PipelineStateObject::SetRootSignature(RootSignature _rootSignature) {
    rootSignature_ = std::move(_rootSignature);
    return *this;
}

PipelineStateObject PipelineStateObject::SetInputLayout(InputLayout _inputLayout) {
    inputLayout_ = std::move(_inputLayout);
    return *this;
}

void PipelineStateObject::Create() {
    desc_.pRootSignature = rootSignature_.Get();
    desc_.InputLayout = inputLayout_.Get();
    // Setting more options
    
    HRESULT hr = adapter_->GetDevice()->CreateGraphicsPipelineState(&desc_, IID_PPV_ARGS(&pso_));
    if (FAILED(hr)){
        Log::Send(Log::Level::ERR, "Failed to create pipeline state object");
        Utils::Alert("Failed to create pipeline state object");
        return;
    } 
    Log::Send(Log::Level::INFO, "Pipeline state object created successfully");
}
