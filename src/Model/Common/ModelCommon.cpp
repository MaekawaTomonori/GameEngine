#include "ModelCommon.hpp"

void ModelCommon::Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi) {
	Setup(_adapter, _debugUi, GraphicsPipeline::Type::MODEL);
}

void ModelCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, ResourceRepository* _resource) {
    resource_ = _resource;
    Initialize(_adapter, _debugUi);
}
