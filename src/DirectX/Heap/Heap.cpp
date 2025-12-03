#include "Heap.hpp"

#include "include/Utils.hpp"

bool Heap::Create(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, UINT _numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags) {
    device_ = _device;
    type_ = _type;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = _type;
    desc.NumDescriptors = _numDescriptors;
    desc.Flags = _flags;
    if (FAILED(device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap_)))){
        Utils::Alert("Failed to create descriptor heap");
        return false;
    }

    return true;
}

ID3D12DescriptorHeap * Heap::Get() const {
    return heap_.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Heap::GetCPUHandle(uint32_t _index) const {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = heap_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(device_->GetDescriptorHandleIncrementSize(type_)) * index;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE Heap::GetGPUHandle(uint32_t _index) const {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = heap_->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(device_->GetDescriptorHandleIncrementSize(type_)) * index;
    return handle;
}
