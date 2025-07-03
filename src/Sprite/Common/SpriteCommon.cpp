#include "SpriteCommon.hpp"

void SpriteCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) {
    Setup(_adapter, _debugUi, GraphicsPipeline::Type::SPRITE);
}

