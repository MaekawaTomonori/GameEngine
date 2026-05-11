#ifndef PipelineStateObject_HPP_
#define PipelineStateObject_HPP_
#include <d3d12.h>
#include <wrl/client.h>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/RootSignature/BlendMode.hpp"
#include "src/DirectX/RootSignature/InputLayout.hpp"
#include "src/DirectX/RootSignature/RootSignature.hpp"
#include "src/DirectX/Shader/Shader.h"

class PipelineStateObject {
    GESTD::ReferencePtr<DirectXAdapter> adapter_ = nullptr;

    RootSignature rootSignature_;
    InputLayout inputLayout_;

    std::unique_ptr<Shader> shader_;

    D3D12_BLEND_DESC blendDesc_{};

    D3D12_RASTERIZER_DESC rasterizer_{
        .FillMode = D3D12_FILL_MODE_SOLID,
        .CullMode = D3D12_CULL_MODE_BACK,
    };

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_ = {};

    D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType_ = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    DXGI_FORMAT rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    UINT numRenderTargets_ = 1;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_;

public:
    PipelineStateObject() = delete;
    PipelineStateObject(GESTD::ReferencePtr<DirectXAdapter> _adapter);
    PipelineStateObject& SetRootSignature(const RootSignature& _rootSignature);
    PipelineStateObject& SetInputLayout(const InputLayout& _inputLayout);
    PipelineStateObject& SetBlend(const D3D12_BLEND_DESC& _blendDesc);
    PipelineStateObject& SetBlend(BlendMode _blendMode);
    PipelineStateObject& SetRasterizer(const D3D12_RASTERIZER_DESC _rasterizer);
    PipelineStateObject& SetDepthStencil(const D3D12_DEPTH_STENCIL_DESC& _depthStencilDesc);
    PipelineStateObject& SetShader(std::unique_ptr<Shader> _shader);
    PipelineStateObject& SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE _topologyType);
    PipelineStateObject& SetDSVFormat(DXGI_FORMAT _format);
    PipelineStateObject& SetRTVFormat(DXGI_FORMAT _format, UINT _numTargets = 1);

    void Create();
    void DrawCall() const;
}; // class PipelineStateObject

#endif // PipelineStateObject_HPP_
