#include "LineCommon.hpp"

void LineCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) {
    Setup(_adapter, _debugUi, GraphicsPipeline::Type::LINE);
}

void LineCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, SRVManager* _srv) {
    srv_ = _srv;
    Setup(_adapter, _debugUi, GraphicsPipeline::Type::LINE);
}