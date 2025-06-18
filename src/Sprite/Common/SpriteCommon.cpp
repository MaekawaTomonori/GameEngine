#include "SpriteCommon.hpp"

void SpriteCommon::Initialize(DirectXAdapter* _adapter) {
    adapter_ = _adapter;

    pipeline_ = std::make_unique<GraphicsPipeline>();
    pipeline_->Create(_adapter, GraphicsPipeline::Type::SPRITE);
}

void SpriteCommon::Draw() const {
    if (pipeline_){
        pipeline_->DrawCall();
    }
}
