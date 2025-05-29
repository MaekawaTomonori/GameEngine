#ifndef Heap_HPP_
#define Heap_HPP_
#include <d3d12.h>
#include <wrl/client.h>

class Heap {
	ID3D12Device* device_ = nullptr; // Assume this is set elsewhere, or pass it in the constructor
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;

public:
	bool Create(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, UINT _numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	ID3D12DescriptorHeap* Get() const;
}; // class Heap

#endif // Heap_HPP_
