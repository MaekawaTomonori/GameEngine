#ifndef Heap_HPP_
#define Heap_HPP_
#include <d3d12.h>
#include <inttypes.h>
#include <wrl/client.h>

/// <summary>
/// ディスクリプタヒープクラス
/// DirectX12のディスクリプタヒープを管理
/// </summary>
class Heap {
    ID3D12Device* device_ = nullptr; // Assume this is set elsewhere, or pass it in the constructor
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
    D3D12_DESCRIPTOR_HEAP_TYPE type_ = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Default type

public:
    bool Create(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, UINT _numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    ID3D12DescriptorHeap* Get() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;
}; // class Heap

#endif // Heap_HPP_
