#ifndef IPostEffect_HPP_
#define IPostEffect_HPP_
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"

class IPostEffect {
protected:
    const Vector4 CLEAR_COLOR {.x= 0, .y= 0, .z= 0, .w= 0};

    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;

    std::unique_ptr<PipelineStateObject> pso_;
    std::unique_ptr<DX12Resource> output_;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};

    uint32_t index_{};
    D3D12_GPU_DESCRIPTOR_HANDLE handle_{};

public:
    IPostEffect(DirectXAdapter* _adapter, SRVManager* _srv);
    virtual ~IPostEffect() = default;
    virtual void Initialize() = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE Apply(D3D12_GPU_DESCRIPTOR_HANDLE _handle);
    void SetRTVHandle(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
    virtual void Debug() = 0;

protected:
    void CreateOutput();
    virtual void Modifier() = 0;
}; // class IPostEffect

#endif // IPostEffect_HPP_
