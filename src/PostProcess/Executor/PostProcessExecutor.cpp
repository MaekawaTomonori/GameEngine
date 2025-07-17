#include "PostProcessExecutor.hpp"

#include "Log.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/GraphicsPipeline/Object/InputLayout.hpp"
#include "src/DirectX/Heap/Heap.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/PostProcess/IPostEffect.hpp"

void PostProcessExecutor::Initialize(DirectXAdapter* _adapter) {
    adapter_ = _adapter;
    
    if (!adapter_) {
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return;
    }

    // シーン用RenderTextureを作成
    CreateSceneRenderTexture();

    //Create PSO
    pso_ = std::make_unique<PipelineStateObject>(adapter_);
    pso_->SetRootSignature(
        RootSignature().SetParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
            .Descriptor = {.ShaderRegister = 0, .RegisterSpace = 0 },
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
        })
    ).Create();
}

void PostProcessExecutor::Add(std::unique_ptr<IPostEffect> _effect) {
    if (_effect){
        effects_.emplace_back(std::move(_effect));
    } else{
        Log::Send(Log::Level::ERR, "Attempted to add a null post effect");
    }
}

void PostProcessExecutor::BeginSceneCapture() {
    if (!adapter_ || !sceneRenderTexture_) {
        Log::Send(Log::Level::ERR, "PostProcessExecutor is not properly initialized");
        return;
    }
    
    // シーンRenderTextureをRenderTargetに設定
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = sceneRenderTexture_.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ;
    
    adapter_->GetCommandList()->ResourceBarrier(1, &barrier);
    
    // 深度ステンシルビューを取得してセット
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = adapter_->GetDSVHandle();
    adapter_->GetCommandList()->OMSetRenderTargets(1, &sceneRTV_, false, &dsvHandle);
    
    // クリア
    Vector4 clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
    adapter_->GetCommandList()->ClearRenderTargetView(sceneRTV_, &clearColor.x, 0, nullptr);

    adapter_->PreProcess();
}

void PostProcessExecutor::EndSceneCapture() {
    if (!adapter_ || !sceneRenderTexture_) {
        return;
    }
    
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = sceneRenderTexture_.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    
    adapter_->GetCommandList()->ResourceBarrier(1, &barrier);
}

void PostProcessExecutor::Execute() {
    if (!adapter_) {
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return;
    }
    
    // エフェクトチェーン処理
    for (const auto& effect : effects_) {
        effect->Apply();
    }
}

void PostProcessExecutor::Draw() {
    if (!adapter_) {
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return;
    }
    
    // CopyImgシェーダーを使用してフルスクリーンクワッドを三角形で描画
    // シーンテクスチャをスワップチェーンに描画
    RenderFullscreenQuad();
}

void PostProcessExecutor::CreateSceneRenderTexture() {
    if (!adapter_) {
        return;
    }

    // RTVHeapを作成
    rtvHeap_ = std::make_unique<Heap>();
    if (!rtvHeap_->Create(adapter_->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 4, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)) {
        Log::Send(Log::Level::ERR, "Failed to create RTV heap for PostProcessExecutor");
        return;
    }

    // 画面サイズのRenderTextureを作成
    Vector4 clearColor{0.0f, 0.0f, 0.0f, 0.0f};
    sceneRenderTexture_.Attach(adapter_->CreateRenderTextureResource(
        static_cast<uint32_t>(adapter_->GetWidth()), 
        static_cast<uint32_t>(adapter_->GetHeight()), 
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 
        clearColor
    ));
    
    if (!sceneRenderTexture_) {
        Log::Send(Log::Level::ERR, "Failed to create scene render texture");
        return;
    }
    
    // RTVを作成
    sceneRTV_ = rtvHeap_->GetCPUHandle(0);
    
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    
    adapter_->GetDevice()->CreateRenderTargetView(sceneRenderTexture_.Get(), &rtvDesc, sceneRTV_);
    
    Log::Send(Log::Level::INFO, "PostProcessExecutor scene render texture created successfully");
}

void PostProcessExecutor::RenderFullscreenQuad() {
    if (!adapter_) {
        return;
    }
    
    // CopyImgシェーダーでフルスクリーンクワッドを三角形描画
    // TODO: 実際のCopyImgシェーダーのPSOとRootSignatureを設定
    // TODO: シーンテクスチャをSRVとして設定
    adapter_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}
