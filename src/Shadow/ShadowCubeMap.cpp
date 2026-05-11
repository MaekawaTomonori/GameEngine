#include "ShadowCubeMap.hpp"

#include <wrl/client.h>

void ShadowCubeMap::Initialize(DirectXAdapter* _adapter, SRVManager* _srv) {
    adapter_ = _adapter;
    srv_     = _srv;

    ID3D12Device* device = adapter_->GetDevice();

    {
        D3D12_RESOURCE_DESC desc{};
        desc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width            = RESOLUTION;
        desc.Height           = RESOLUTION;
        desc.DepthOrArraySize = FACE_COUNT;
        desc.MipLevels        = 1;
        desc.Format           = DXGI_FORMAT_R32_FLOAT;
        desc.SampleDesc.Count = 1;
        desc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_HEAP_PROPERTIES heapProps{};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_CLEAR_VALUE clearValue{};
        clearValue.Format   = DXGI_FORMAT_R32_FLOAT;
        clearValue.Color[0] = 1.0f;
        clearValue.Color[1] = 1.0f;
        clearValue.Color[2] = 1.0f;
        clearValue.Color[3] = 1.0f;

        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            &clearValue,
            IID_PPV_ARGS(&resource)
        );

        cubeRT_ = std::make_unique<DX12Resource>();
        cubeRT_->Create(resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    {
        D3D12_RESOURCE_DESC desc{};
        desc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width            = RESOLUTION;
        desc.Height           = RESOLUTION;
        desc.DepthOrArraySize = 1;
        desc.MipLevels        = 1;
        desc.Format           = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count = 1;
        desc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_HEAP_PROPERTIES heapProps{};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_CLEAR_VALUE clearValue{};
        clearValue.Format               = DXGI_FORMAT_D32_FLOAT;
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&resource)
        );

        depth_ = std::make_unique<DX12Resource>();
        depth_->Create(resource, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }

    {
        rtvHeap_ = std::make_unique<Heap>();
        rtvHeap_->Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FACE_COUNT);

        for (UINT i = 0; i < FACE_COUNT; ++i) {
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
            rtvDesc.Format                         = DXGI_FORMAT_R32_FLOAT;
            rtvDesc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice        = 0;
            rtvDesc.Texture2DArray.FirstArraySlice = i;
            rtvDesc.Texture2DArray.ArraySize       = 1;
            rtvDesc.Texture2DArray.PlaneSlice      = 0;

            device->CreateRenderTargetView(cubeRT_->Get(), &rtvDesc, rtvHeap_->GetCPUHandle(i));
        }
    }

    {
        dsvHeap_ = std::make_unique<Heap>();
        dsvHeap_->Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format             = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        device->CreateDepthStencilView(depth_->Get(), &dsvDesc, dsvHeap_->GetCPUHandle(0));
    }

    srvIndex_ = srv_->Allocate();
    srv_->CreateSRVForCubeMap(srvIndex_, cubeRT_->Get(), DXGI_FORMAT_R32_FLOAT);
}

void ShadowCubeMap::TransitionToRenderTarget(ID3D12GraphicsCommandList* _cmdList) const {
    cubeRT_->ChangeState(_cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void ShadowCubeMap::TransitionToShaderResource(ID3D12GraphicsCommandList* _cmdList) const {
    cubeRT_->ChangeState(_cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void ShadowCubeMap::ClearFace(ID3D12GraphicsCommandList* _cmdList, UINT _face) const {
    float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    _cmdList->ClearRenderTargetView(rtvHeap_->GetCPUHandle(_face), clearColor, 0, nullptr);
    _cmdList->ClearDepthStencilView(dsvHeap_->GetCPUHandle(0), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void ShadowCubeMap::SetFaceAsRenderTarget(ID3D12GraphicsCommandList* _cmdList, UINT _face) const {
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtvHeap_->GetCPUHandle(_face);
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = dsvHeap_->GetCPUHandle(0);
    _cmdList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
}

D3D12_CPU_DESCRIPTOR_HANDLE ShadowCubeMap::GetDSVHandle() const {
    return dsvHeap_->GetCPUHandle(0);
}
