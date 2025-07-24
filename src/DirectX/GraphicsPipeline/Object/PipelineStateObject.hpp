#ifndef PipelineStateObject_HPP_
#define PipelineStateObject_HPP_
#include <d3d12.h>
#include <wrl/client.h>

#include "BlendMode.hpp"
#include "InputLayout.hpp"
#include "RootSignature.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Shader/Shader.h"

class PipelineStateObject {
    DirectXAdapter* adapter_ = nullptr;

    RootSignature rootSignature_;
    InputLayout inputLayout_;

    std::unique_ptr<Shader> shader_;

    D3D12_BLEND_DESC blendDesc_{};

    D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType_ = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_;

public:
    PipelineStateObject() = delete;
    PipelineStateObject(DirectXAdapter* _adapter);
    PipelineStateObject& SetRootSignature(const RootSignature& _rootSignature);
    PipelineStateObject& SetInputLayout(const InputLayout& _inputLayout);
    PipelineStateObject& SetBlend(const D3D12_BLEND_DESC& _blendDesc);
    PipelineStateObject& SetBlend(BlendMode _blendMode);
    PipelineStateObject& SetShader(std::unique_ptr<Shader> _shader);
    PipelineStateObject& SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE _topologyType);

    void Create();
    void DrawCall() const;
}; // class PipelineStateObject

#endif // PipelineStateObject_HPP_
