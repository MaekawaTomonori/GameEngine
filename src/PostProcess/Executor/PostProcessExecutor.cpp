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

    D3D12_DESCRIPTOR_RANGE range{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

    //Create PSO
    pso_ = std::make_unique<PipelineStateObject>(adapter_);
    pso_->SetRootSignature(
        RootSignature()
        .SetParameter({
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
            .DescriptorTable = {
                .NumDescriptorRanges = 1,
                .pDescriptorRanges = &range
            },
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
        })
        .SetSampler({
            .Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
            .MaxLOD = D3D12_FLOAT32_MAX,
            .ShaderRegister = 0,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
        })
    )
    .SetShader(std::make_unique<Shader>(L"CpyImg"))
    .Create();
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
    
    // RenderTextureは既にRENDER_TARGET状態で作成されているので、バリアは不要
    // 深度ステンシルビューを取得してセット
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = adapter_->GetDSVHandle();
    adapter_->GetCommandList()->OMSetRenderTargets(1, &sceneRTV_, false, &dsvHandle);
    
    // クリア
    Vector4 clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
    adapter_->GetCommandList()->ClearRenderTargetView(sceneRTV_, &clearColor.x, 0, nullptr);
    
    // ViewportとScissorを設定
    D3D12_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(adapter_->GetWidth());
    viewport.Height = static_cast<float>(adapter_->GetHeight());
    viewport.MaxDepth = 1.0f;
    
    D3D12_RECT scissor = {};
    scissor.right = static_cast<LONG>(adapter_->GetWidth());
    scissor.bottom = static_cast<LONG>(adapter_->GetHeight());
    
    adapter_->GetCommandList()->RSSetViewports(1, &viewport);
    adapter_->GetCommandList()->RSSetScissorRects(1, &scissor);
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
    if (!adapter_ || !pso_) {
        Log::Send(Log::Level::ERR, "PostProcessExecutor is not properly initialized");
        return;
    }
    
    // PSO設定
    pso_->DrawCall();
    
    // SRVを設定（TODO: 実際のSRVが作成されたら有効化）
    // adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(0, sceneSRV_);
    
    // フルスクリーンクワッドを三角形で描画
    adapter_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
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
    
    // SRVを作成（SRVManagerを使用する必要があるが、今は簡易実装）
    // TODO: 実際にはSRVManagerを使用してSRVを作成し、sceneSRV_に設定
    // sceneSRV_ = srvManager->CreateSRVForTexture2D(...);
    
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
