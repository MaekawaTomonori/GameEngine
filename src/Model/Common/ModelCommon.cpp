#include "ModelCommon.hpp"

void ModelCommon::Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi) {
	Setup(_adapter, _debugUi, GraphicsPipeline::Type::MODEL);
}
