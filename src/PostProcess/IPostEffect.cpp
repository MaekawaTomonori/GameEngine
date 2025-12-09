#include "IPostEffect.hpp"

#include <d3d12.h>

#include "Utils.hpp"

void IPostEffect::SetUp(DirectXAdapter* _adapter, SRVManager* _srv) {
    adapter_ = _adapter;
    srv_ = _srv;
    CreateOutput();
}


D3D12_GPU_DESCRIPTOR_HANDLE IPostEffect::Apply(const D3D12_GPU_DESCRIPTOR_HANDLE _handle) {
    if (!output_) Utils::Alert("");

    output_->ChangeState(adapter_->GetCommandList(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    pso_->DrawCall();

    adapter_->GetCommandList()->OMSetRenderTargets(1, &rtvHandle_, false, nullptr);
    adapter_->GetCommandList()->ClearRenderTargetView(rtvHandle_, &CLEAR_COLOR.x, 0, nullptr);

    adapter_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(0, _handle);

    Modifier();

    adapter_->GetCommandList()->DrawInstanced(3, 1, 0, 0);

    output_->ChangeState(adapter_->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    return srv_->GetGPUHandle(index_);
}

void IPostEffect::CreateOutput() {
    output_ = adapter_->CreateRenderTextureResource(static_cast<uint32_t>(adapter_->GetWidth()), static_cast<uint32_t>(adapter_->GetHeight()), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, CLEAR_COLOR);

    index_ = srv_->Allocate();
    srv_->CreateSRVforTexture2D(index_, output_->Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);
    handle_ = srv_->GetGPUHandle(index_);
}

void IPostEffect::SetRTVHandle(D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle) {
    rtvHandle_ = _rtvHandle;

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    adapter_->GetDevice()->CreateRenderTargetView(output_->Get(), &rtvDesc, rtvHandle_);
}
