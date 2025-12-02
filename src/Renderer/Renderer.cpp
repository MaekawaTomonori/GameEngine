#include "Renderer.hpp"

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/PostProcess/Executor/PostProcessExecutor.hpp"

void Renderer::Initialize(DirectXAdapter* _adapter, PostProcessExecutor* _postProcessor) {
    adapter_ = _adapter;
    postProcessor_ = _postProcessor;
}

void Renderer::Register(const std::function<void()>& _task, const bool _applyPostEffect) {
    if (_applyPostEffect){
        pp_.push(std::move(_task));
    } else{
        tasks_.push(std::move(_task));
    }
}

void Renderer::Render() {
    std::function<void()> ppFunc;
    if (!pp_.empty()){
        postProcessor_->BeginFrame();
        while (!pp_.empty()) {
            if (pp_.empty()) break;
            auto task = std::move(pp_.front());
            pp_.pop();
            task();
        }
        postProcessor_->EndFrame();
        postProcessor_->Execute();

        ppFunc = ([this](){postProcessor_->Draw();});
    }

    if (!tasks_.empty()) {
        adapter_->BeginFrame();
        if (ppFunc) {
            ppFunc();
        }
        while (!tasks_.empty()){
            auto task = std::move(tasks_.front());
            tasks_.pop();
            task();
        }
        adapter_->EndFrame();
    }
}