#include "DX12Resource.hpp"

#include "Utils.hpp"

void DX12Resource::Create(ID3D12Resource* _resource, D3D12_RESOURCE_STATES _state) {
    resource_.Attach(_resource);
    if (!resource_){
        Utils::Alert("Failed to create DX12 Resource");
        return;
    }
    state_ = _state;
}

void DX12Resource::Create(const Microsoft::WRL::ComPtr<ID3D12Resource>& _resource, D3D12_RESOURCE_STATES _state) {
    resource_ = _resource;
    if (!resource_){
        Utils::Alert("Failed to create DX12 Resource");
        return;
    }
    state_ = _state;
}

void DX12Resource::ChangeState(ID3D12GraphicsCommandList* _command, const D3D12_RESOURCE_STATES _state) {
    if (!resource_) return;
    if (state_ == _state) return;
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource_.Get();
    barrier.Transition.StateBefore = state_;
    barrier.Transition.StateAfter = _state;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    _command->ResourceBarrier(1, &barrier);

    state_ = _state;
}

ID3D12Resource* DX12Resource::Get() const {
    return resource_.Get();
}
