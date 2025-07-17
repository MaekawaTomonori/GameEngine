#ifndef PostProcessExecutor_HPP_
#define PostProcessExecutor_HPP_
#include <vector>
#include <memory>
#include <wrl/client.h>
#include <d3d12.h>

#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/PostProcess/IPostEffect.hpp"

class DirectXAdapter;
class Heap;

class PostProcessExecutor {
    DirectXAdapter* adapter_ = nullptr;
    std::vector<std::unique_ptr<IPostEffect>> effects_;
    
    // シーン描画用RenderTexture
    Microsoft::WRL::ComPtr<ID3D12Resource> sceneRenderTexture_;
    std::unique_ptr<Heap> rtvHeap_;
    D3D12_CPU_DESCRIPTOR_HANDLE sceneRTV_{};
    D3D12_GPU_DESCRIPTOR_HANDLE sceneSRV_{};

    std::unique_ptr<PipelineStateObject> pso_;
public:
    void Initialize(DirectXAdapter* _adapter);
    void Add(std::unique_ptr<IPostEffect> _effect);
    
    // シーンキャプチャ用メソッド
    void BeginSceneCapture();
    void EndSceneCapture();
    void Execute();
    void Draw();

private:
    void CreateSceneRenderTexture();
    void RenderFullscreenQuad();
}; // class PostProcessExecutor

#endif // PostProcessExecutor_HPP_
