#ifndef DX12Resource_HPP_
#define DX12Resource_HPP_
#include <wrl/client.h>
#include <d3d12.h>

class DX12Resource {
    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
    D3D12_RESOURCE_STATES state_ = D3D12_RESOURCE_STATE_COMMON;
public:
    void Create(ID3D12Resource* _resource, D3D12_RESOURCE_STATES _state = D3D12_RESOURCE_STATE_COMMON);
    void ChangeState(ID3D12GraphicsCommandList* _command, D3D12_RESOURCE_STATES _state);
    ID3D12Resource* Get() const;
}; // class DX12Resource

#endif // DX12Resource_HPP_
