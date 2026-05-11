#include "ShadowPass.hpp"

#include <DirectXMath.h>

#include "src/DirectX/RootSignature/BlendMode.hpp"
#include "src/DirectX/RootSignature/InputLayout.hpp"
#include "src/DirectX/RootSignature/RootSignature.hpp"
#include "src/DirectX/Shader/Shader.h"

void ShadowPass::Initialize(DirectXAdapter* _adapter) {
    adapter_ = _adapter;

    for (UINT i = 0; i < ShadowCubeMap::FACE_COUNT; ++i) {
        shadowCameraResources_[i] = adapter_->CreateBufferResource(sizeof(ShadowCamera));
        shadowCameraResources_[i]->Get()->Map(0, nullptr, reinterpret_cast<void**>(&shadowCameras_[i]));
        shadowCameras_[i]->farPlane = 1.0f;
    }

    shadowDataResource_ = adapter_->CreateBufferResource(sizeof(ShadowData));
    shadowDataResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&shadowData_));

    shadowData_->farPlane = 1.0f;

    pso_ = std::make_unique<PipelineStateObject>(GESTD::ReferencePtr<DirectXAdapter>(_adapter));

    pso_->SetRootSignature(
        RootSignature()
            .AddParameter({
                .ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor       = { .ShaderRegister = 0 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
            })
            .AddParameter({
                .ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor       = { .ShaderRegister = 1 },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
            })
    )
    .SetInputLayout(
        InputLayout{}
            .SetElement({"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
            .SetElement({"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
            .SetElement({"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
    )
    .SetBlend(D3D12_BLEND_DESC{
        .AlphaToCoverageEnable  = false,
        .IndependentBlendEnable = false,
        .RenderTarget           = {{
            .BlendEnable           = false,
            .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
        }}
    })
    .SetDepthStencil({
        .DepthEnable      = true,
        .DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL,
        .DepthFunc        = D3D12_COMPARISON_FUNC_LESS_EQUAL,
        .StencilEnable    = false
    })
    .SetDSVFormat(DXGI_FORMAT_D32_FLOAT)
    .SetRTVFormat(DXGI_FORMAT_R32_FLOAT, 1)
    .SetShader(std::make_unique<Shader>(L"PointShadow"))
    .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
    .Create();
}

void ShadowPass::Execute(ShadowCubeMap& _cubeMap, ModelCommon& _model, LightManager& _light) {
    const PointLight* pl = _light.GetShadowCastingPointLight();
    if (!pl) return;

    using namespace DirectX;

    ID3D12GraphicsCommandList* cmd = adapter_->GetCommandList();

    shadowData_->lightPos = pl->position;
    shadowData_->farPlane = pl->radius;

    D3D12_VIEWPORT vp{
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width    = static_cast<float>(ShadowCubeMap::RESOLUTION),
        .Height   = static_cast<float>(ShadowCubeMap::RESOLUTION),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f
    };
    D3D12_RECT scissor{
        .left   = 0,
        .top    = 0,
        .right  = static_cast<LONG>(ShadowCubeMap::RESOLUTION),
        .bottom = static_cast<LONG>(ShadowCubeMap::RESOLUTION)
    };

    XMVECTOR eyePos = XMVectorSet(pl->position.x, pl->position.y, pl->position.z, 1.0f);
    XMMATRIX proj   = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.1f, pl->radius);

    struct FaceDesc { XMFLOAT3 dir; XMFLOAT3 up; };
    static const FaceDesc kFaces[ShadowCubeMap::FACE_COUNT] = {
        {{ 1,  0,  0}, { 0,  1,  0}},
        {{-1,  0,  0}, { 0,  1,  0}},
        {{ 0,  1,  0}, { 0,  0, -1}},
        {{ 0, -1,  0}, { 0,  0,  1}},
        {{ 0,  0,  1}, { 0,  1,  0}},
        {{ 0,  0, -1}, { 0,  1,  0}},
    };

    _cubeMap.TransitionToRenderTarget(cmd);
    pso_->DrawCall();
    cmd->RSSetViewports(1, &vp);
    cmd->RSSetScissorRects(1, &scissor);

    for (UINT face = 0; face < ShadowCubeMap::FACE_COUNT; ++face) {
        XMVECTOR dir      = XMLoadFloat3(&kFaces[face].dir);
        XMVECTOR up       = XMLoadFloat3(&kFaces[face].up);
        XMMATRIX viewProj = XMMatrixMultiply(XMMatrixLookToLH(eyePos, dir, up), proj);

        shadowCameras_[face]->lightPos = pl->position;
        shadowCameras_[face]->farPlane = pl->radius;
        XMStoreFloat4x4(
            reinterpret_cast<XMFLOAT4X4*>(&shadowCameras_[face]->faceVP),
            viewProj
        );

        _cubeMap.ClearFace(cmd, face);
        _cubeMap.SetFaceAsRenderTarget(cmd, face);

        cmd->SetGraphicsRootConstantBufferView(1, shadowCameraResources_[face]->Get()->GetGPUVirtualAddress());
        _model.ExecuteShadowDraw();
    }

    _cubeMap.TransitionToShaderResource(cmd);
}

D3D12_GPU_VIRTUAL_ADDRESS ShadowPass::GetShadowDataAddress() const {
    return shadowDataResource_->Get()->GetGPUVirtualAddress();
}
