#include "Heap.hpp"

#include <algorithm>

#include "Utils.hpp"

bool Heap::Create(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, UINT _numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags) {
    device_ = _device;
    type_ = _type;
    numDescriptors_ = _numDescriptors;
    nextIndex_ = 0;
    freeIndices_.clear();

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

uint32_t Heap::Allocate() {
    if (!freeIndices_.empty()) {
        uint32_t index = freeIndices_.back();
        freeIndices_.pop_back();
        return index;
    }

    if (nextIndex_ >= numDescriptors_) {
        Utils::Alert("Heap is full");
        return 0;
    }

    return nextIndex_++;
}

void Heap::Free(uint32_t _index) {
    if (_index >= nextIndex_) {
        Utils::Alert("Heap::Free: index out of range");
        return;
    }

#ifdef _DEBUG
    if (std::find(freeIndices_.begin(), freeIndices_.end(), _index) != freeIndices_.end()) {
        Utils::Alert("Heap::Free: double free detected");
        return;
    }
#endif

    freeIndices_.push_back(_index);
}

bool Heap::IsFull() const {
    return freeIndices_.empty() && nextIndex_ >= numDescriptors_;
}

ID3D12DescriptorHeap * Heap::Get() const {
    return heap_.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Heap::GetCPUHandle(uint32_t _index) const {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = heap_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(device_->GetDescriptorHandleIncrementSize(type_)) * _index;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE Heap::GetGPUHandle(uint32_t _index) const {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = heap_->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(device_->GetDescriptorHandleIncrementSize(type_)) * _index;
    return handle;
}
