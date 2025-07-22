#ifndef PostProcessExecutor_HPP_
#define PostProcessExecutor_HPP_
#include <vector>
#include <memory>
#include <d3d12.h>

#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/PostProcess/IPostEffect.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

class DirectXAdapter;
class Heap;

class PostProcessExecutor {
    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;

    std::vector<std::unique_ptr<IPostEffect>> effects_;
    
    // シーン描画用RenderTexture
    std::unique_ptr<DX12Resource> renderTexture_;
    std::unique_ptr<Heap> rtvHeap_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle_{};

    std::unique_ptr<PipelineStateObject> pso_;

    Vector4 clearColor_{ 0.0f, 0.0f, 0.0f, 0.0f };
    uint32_t srvIndex_ = 0;

public:
    void Initialize(DirectXAdapter* _adapter, SRVManager* _srv);
    void Add(std::unique_ptr<IPostEffect> _effect);
    
    void BeginFrame() const;
    void EndFrame() const;
    void Execute() const;
    void Draw() const;

private:
    void CreateSceneRenderTexture();
}; // class PostProcessExecutor

#endif // PostProcessExecutor_HPP_
