#include "ModelCommon.hpp"

#include "src/DirectX/Heap/SRVManager.h"

void ModelCommon::Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi) {
	Setup(_adapter, _debugUi, GraphicsPipeline::Type::SKINNING_MODEL);
}

void ModelCommon::Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, ResourceRepository* _resource, SRVManager* _srv) {
    resource_ = _resource;
    srv_ = _srv;
    Initialize(_adapter, _debugUi);
}
