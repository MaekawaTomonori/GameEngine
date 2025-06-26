#include "SpriteCommon.hpp"

void SpriteCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) {
    std::lock_guard<std::mutex> lock(mutex_);
    adapter_ = _adapter;
    debugUI_ = _debugUi;

    pipeline_ = std::make_unique<GraphicsPipeline>();
    pipeline_->Create(_adapter, GraphicsPipeline::Type::SPRITE);
}

void SpriteCommon::Draw() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pipeline_){
        pipeline_->DrawCall();
    }
}

void SpriteCommon::RegisterCommand(const std::string &_id, const std::function<void()> &_command) {
	std::lock_guard<std::mutex> lock(mutex_);
	if (debugUI_){
		debugUI_->RegisterCommand(_id, _command);
	}
}
