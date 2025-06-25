#include "SRVManager.h"

#include <cassert>
#include <memory>

#include "Heap.hpp"
#include "Log.hpp"

const uint32_t SRVManager::kMaxSRVCount = 512;

void SRVManager::Initialize(DirectXAdapter* _adapter) {
    adapter_ = _adapter;

	auto dxc = adapter_;
    if (!dxc){
        Log::Send(Log::Level::ERR, "SRVManager Initialize Failed");
        return;
    }

    heap_ = std::make_shared<Heap>();
    heap_->Create(dxc->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    descriptorSize = dxc->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    useIndex_ = 0;

    Log::Send(Log::Level::INFO, "SRVManager Enabled");
}

void SRVManager::Finalize() {
    //Log::Send((Log::Level::INFO, "SRVManager Disabled");
}

uint32_t SRVManager::Allocate() {
    assert(useIndex_ <= kMaxSRVCount);

    uint32_t index = useIndex_;

    ++useIndex_;

    return index;
}

void SRVManager::PreDraw() const {
    ID3D12DescriptorHeap* descriptorHeaps[] = {heap_->Get()};
    adapter_->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
}

void SRVManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipMap) {
    D3D12_SHADER_RESOURCE_VIEW_DESC desc {};
    desc.Format = format;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipLevels = mipMap;

    adapter_->GetDevice()->CreateShaderResourceView(pResource, &desc, heap_->GetCPUHandle(srvIndex));
}

void SRVManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT stride) {
    D3D12_SHADER_RESOURCE_VIEW_DESC desc {};
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    desc.Buffer.NumElements = numElements;
    desc.Buffer.StructureByteStride = stride;
    adapter_->GetDevice()->CreateShaderResourceView(pResource, &desc, heap_->GetCPUHandle(srvIndex));
}

void SRVManager::SetGraphicsRootDescriptorTable(UINT rootParameterIndex, uint32_t srvIndex) const {
    adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(rootParameterIndex, heap_->GetGPUHandle(srvIndex));
}

ID3D12DescriptorHeap* SRVManager::GetDescriptorHeap() const {
	return heap_->Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE SRVManager::GetCPUHandle(uint32_t index) const {
    assert(index <= useIndex_);
    return heap_->GetCPUHandle(index);
}

D3D12_GPU_DESCRIPTOR_HANDLE SRVManager::GetGPUHandle(uint32_t index) const {
    assert(index <= useIndex_);
    return heap_->GetGPUHandle(index);
}
