#include "PostProcessExecutor.hpp"

#include "Log.hpp"

void PostProcessExecutor::Initialize(DirectXAdapter* _adapter) {
}

void PostProcessExecutor::Add(std::unique_ptr<IPostEffect> _effect) {
    if (_effect){
        effects_.emplace_back(std::move(_effect));
    } else{
        Log::Send(Log::Level::ERR, "Attempted to add a null post effect");
    }
}

void PostProcessExecutor::Execute() const {
    if (!effects_.empty()) {
        for (const auto& effect : effects_) {
            effect->Apply();
        }
    }

    ToSwapChain();
}

void PostProcessExecutor::ToSwapChain() const{
    if (!adapter_){
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return;
    }

    adapter_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}
