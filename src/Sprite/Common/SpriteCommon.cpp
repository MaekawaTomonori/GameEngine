#include "SpriteCommon.hpp"

void SpriteCommon::Initialize(DirectXAdapter* _adapter) {
	std::lock_guard<std::mutex> lock(mutex_);
	adapter_ = _adapter;
    
    pipeline_ = std::make_unique<GraphicsPipeline>();
    pipeline_->Create(_adapter, GraphicsPipeline::Type::SPRITE);
}

void SpriteCommon::Draw() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pipeline_){
        pipeline_->DrawCall();
    }
}
