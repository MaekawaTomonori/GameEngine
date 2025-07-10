#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "src/DirectX/Shader/Shader.h"

class DirectXAdapter;
class Heap;

enum class BlendMode{
    ALPHA,
    ADD,
    SUB,
    MULTI,
    SCREEN,

    NONE
};

class GraphicsPipeline{
public:
    enum class Type{
        MODEL,
        SKINNING_MODEL,
        SPRITE,
        PARTICLE,
        PARTICLE2D,

        COUNT
    };

    void Create(DirectXAdapter* _adapter, Type _type);

    void DrawCall() const;

    void SetBlendMode(BlendMode mode);

private://Methods
    void CreateRootSignature();
    void DescriptorRange();
    void CreateInputLayout();
    void CreateBlendState();
    void CreateShader();
    void CreateRasterizerState();
    void CreateSampler();
    void CreateDepthStencil();

    void CreatePSO();

private://Variables
    DirectXAdapter* adapter_ = nullptr;

    Type type_ = Type::MODEL;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
    D3D12_DESCRIPTOR_RANGE descriptorRange_[1] {};
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_ {};
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs_{};
    BlendMode blendMode_ = BlendMode::NONE;
    D3D12_BLEND_DESC blendDesc_ {};
    D3D12_RASTERIZER_DESC rasterizerDesc_ {};

    D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};

    std::unique_ptr<Shader> shader_;

    //DepthStencil
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_ {};

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc {};
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
};
