#include "Common.hpp"

void Common::Setup(DirectXAdapter* _adapter, DebugUI* _debugUi, const GraphicsPipeline::Type _type) {
    std::lock_guard<std::mutex> lock(mutex_);
    adapter_ = _adapter;
    debugUI_ = _debugUi;

    pipeline_ = std::make_unique<GraphicsPipeline>();
    pipeline_->Create(adapter_, _type);
}

void Common::Setup(DirectXAdapter* _adapter, DebugUI* _debugUi) {
    std::lock_guard<std::mutex> lock(mutex_);
    adapter_ = _adapter;
    debugUI_ = _debugUi;
}

void Common::Draw() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pipeline_){
        pipeline_->DrawCall();
    }
}

void Common::RegisterCommand(const std::string &_id, const std::function<void()> &_command) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (debugUI_){
        debugUI_->RegisterCommand(_id, _command);
    }
}
