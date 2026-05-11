#pragma once
#include <d3d12.h>
#include <memory>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/Heap.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/Resource/DX12Resource.hpp"

class ShadowCubeMap {
public:
    static constexpr UINT RESOLUTION = 512;
    static constexpr UINT FACE_COUNT = 6;

private:
    std::unique_ptr<DX12Resource> cubeRT_;
    std::unique_ptr<DX12Resource> depth_;

    std::unique_ptr<Heap> rtvHeap_;
    std::unique_ptr<Heap> dsvHeap_;

    uint32_t srvIndex_ = UINT_MAX;

    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;

public:
    void Initialize(DirectXAdapter* _adapter, SRVManager* _srv);

    void TransitionToRenderTarget(ID3D12GraphicsCommandList* _cmdList) const;
    void TransitionToShaderResource(ID3D12GraphicsCommandList* _cmdList) const;

    void ClearFace(ID3D12GraphicsCommandList* _cmdList, UINT _face) const;
    void SetFaceAsRenderTarget(ID3D12GraphicsCommandList* _cmdList, UINT _face) const;

    uint32_t GetSRVIndex() const { return srvIndex_; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const;
};
