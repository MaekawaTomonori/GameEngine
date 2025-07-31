#pragma once
#include <cstdint>
#include <d3d12.h>
#include <memory>

#include "src/DirectX/DirectXAdapter.hpp"

class Heap;

class SRVManager{
    DirectXAdapter* adapter_ = nullptr;

    static const uint32_t kMaxSRVCount;

    uint32_t descriptorSize = 0;
    uint32_t useIndex_ = 0;

    std::shared_ptr<Heap> heap_;

public:
	void Initialize(DirectXAdapter* _adapter);
    void Finalize();

    uint32_t Allocate();
    void PreDraw() const;

    void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipMap);
    void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT stride);
    void CreateSRVforCubemap(uint32_t _srvIndex, ID3D12Resource* _pResource, DXGI_FORMAT _format);

    void SetGraphicsRootDescriptorTable(UINT rootParameterIndex, uint32_t srvIndex) const;

    bool IsFull() const {
        return kMaxSRVCount <= useIndex_;
    }

    ID3D12DescriptorHeap* GetDescriptorHeap() const;

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;
};

