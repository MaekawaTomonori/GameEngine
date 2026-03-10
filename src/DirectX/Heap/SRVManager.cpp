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

void SRVManager::CreateSRVForTexture2D(uint32_t _srvIndex, ID3D12Resource* _pResource, DXGI_FORMAT _format, UINT _mipMap) const {
    D3D12_SHADER_RESOURCE_VIEW_DESC desc {};
    desc.Format = _format;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipLevels = _mipMap;

    adapter_->GetDevice()->CreateShaderResourceView(_pResource, &desc, heap_->GetCPUHandle(_srvIndex));
}

void SRVManager::CreateSRVForStructuredBuffer(uint32_t _srvIndex, ID3D12Resource* _pResource, UINT _numElements, UINT _stride) const {
    D3D12_SHADER_RESOURCE_VIEW_DESC desc {};
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    desc.Buffer.NumElements = _numElements;
    desc.Buffer.StructureByteStride = _stride;
    adapter_->GetDevice()->CreateShaderResourceView(_pResource, &desc, heap_->GetCPUHandle(_srvIndex));
}

void SRVManager::CreateSRVForCubeMap(uint32_t _srvIndex, ID3D12Resource* _pResource, DXGI_FORMAT _format) const {
    D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
    desc.Format = _format;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    desc.TextureCube.MostDetailedMip = 0;
    desc.TextureCube.MipLevels = UINT_MAX;
    desc.TextureCube.ResourceMinLODClamp = 0.f;

    adapter_->GetDevice()->CreateShaderResourceView(_pResource, &desc, heap_->GetCPUHandle(_srvIndex));
}

void SRVManager::SetGraphicsRootDescriptorTable(UINT _rootParameterIndex, uint32_t _srvIndex) const {
    adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(_rootParameterIndex, heap_->GetGPUHandle(_srvIndex));
}

ID3D12DescriptorHeap* SRVManager::GetDescriptorHeap() const {
	return heap_->Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE SRVManager::GetCPUHandle(uint32_t _index) const {
    assert(_index <= useIndex_);
    return heap_->GetCPUHandle(_index);
}

D3D12_GPU_DESCRIPTOR_HANDLE SRVManager::GetGPUHandle(uint32_t _index) const {
    assert(_index <= useIndex_);
    return heap_->GetGPUHandle(_index);
}
