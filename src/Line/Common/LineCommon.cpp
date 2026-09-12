#include "LineCommon.hpp"

#include "src/DirectX/RootSignature/BlendMode.hpp"
#include "src/DirectX/RootSignature/InputLayout.hpp"
#include "src/DirectX/RootSignature/RootSignature.hpp"
#include "src/DirectX/Shader/Shader.h"

void LineCommon::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi) {
    Setup(_adapter, _debugUi, "Line");

    // PipelineStateObjectの初期化
    pipeline_ = std::make_unique<PipelineStateObject>(_adapter);

    // PipelineStateObject作成 (Line用)
    pipeline_->SetRootSignature(
        RootSignature()
            // 1. Material CBV (Pixel Shader, register 0)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = {
                    .ShaderRegister = 0
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            })
            // 2. Transform CBV (Vertex Shader, register 0)
            .AddParameter({
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor = {
                    .ShaderRegister = 0
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
            })
    )
    .SetInputLayout(InputLayout{}// InputLayout設定 (LINE用)
        .SetElement({"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
    )
    .SetBlend(BlendMode::NONE)
    .SetShader(
        // シェーダー設定 (LINE用)
        std::make_unique<Shader>(L"Line")
    )
    .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE)
    .Create();
}

void LineCommon::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi, SRVManager* _srv) {
    srv_ = _srv;
    Initialize(_adapter, _debugUi);
}

GESTD::ReferencePtr<LineInstance> LineCommon::CreateInstance() {
    auto instance = std::make_unique<LineInstance>();
    instance->Initialize();

    GESTD::ReferencePtr<LineInstance> reference = instance->GetReference();
    instances_.push_back(std::move(instance));
    return reference;
}

void LineCommon::DestroyInstance(const GESTD::ReferencePtr<LineInstance>& _instance) {
    LineInstance* raw = _instance;
    if (!raw) return;

    std::erase_if(instances_, [raw](const std::unique_ptr<LineInstance>& _entry) {
        return _entry.get() == raw;
    });
}

void LineCommon::RegisterDraw(LineInstance* _instance) {
    drawQueue_.push_back(_instance);
}

void LineCommon::Draw(Renderer* _renderer) {
    if (drawQueue_.empty()) return;
    if (!pipeline_) return;

    std::lock_guard<std::mutex> lock(mutex_);
    _renderer->Register([this, queue = std::move(drawQueue_)]() {
        pipeline_->DrawCall();
        for (LineInstance* instance : queue) {
            instance->ExecuteDraw();
        }
    }, "None");

    drawQueue_.clear();
}
