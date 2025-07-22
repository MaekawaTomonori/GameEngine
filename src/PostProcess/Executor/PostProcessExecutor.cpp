#include "PostProcessExecutor.hpp"

#include "Log.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/GraphicsPipeline/Object/InputLayout.hpp"
#include "src/DirectX/Heap/Heap.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/PostProcess/IPostEffect.hpp"

void PostProcessExecutor::Initialize(DirectXAdapter* _adapter, SRVManager* _srv) {
    adapter_ = _adapter;
    srv_ = _srv;

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

void PostProcessExecutor::BeginFrame() const {
    if (!adapter_ || !renderTexture_) {
        Log::Send(Log::Level::ERR, "PostProcessExecutor is not properly initialized");
        return;
    }

    renderTexture_->ChangeState(adapter_->GetCommandList(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    auto dsvHandle = adapter_->GetDSVHandle();
    adapter_->GetCommandList()->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle);
    adapter_->GetCommandList()->ClearRenderTargetView(rtvHandle_, &clearColor_.x, 0, nullptr);

    adapter_->PreProcess();
}

void PostProcessExecutor::EndFrame() const {
    if (!adapter_ || !renderTexture_) {
        return;
    }
    
    renderTexture_->ChangeState(adapter_->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void PostProcessExecutor::Execute() const {
    if (!adapter_) {
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return;
    }
    
    // エフェクトチェーン処理
    for (const auto& effect : effects_) {
        effect->Apply();
    }
}

void PostProcessExecutor::Draw() const {
    if (!adapter_ || !pso_) {
        Log::Send(Log::Level::ERR, "PostProcessExecutor is not properly initialized");
        return;
    }

    srv_->PreDraw();

    // PSO設定
    pso_->DrawCall();

    adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(0, srvHandle_);
    
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
    renderTexture_ = std::make_unique<DX12Resource>();
    renderTexture_ = adapter_->CreateRenderTextureResource(
        static_cast<uint32_t>(adapter_->GetWidth()), 
        static_cast<uint32_t>(adapter_->GetHeight()), 
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 
        clearColor_
    );

    renderTexture_->Get()->SetName(L"RenderTexture");

    if (!renderTexture_->Get()) {
        Log::Send(Log::Level::ERR, "Failed to create scene render texture");
        return;
    }
    
    // RTVを作成
    rtvHandle_ = rtvHeap_->GetCPUHandle(0);
    
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    
    adapter_->GetDevice()->CreateRenderTargetView(renderTexture_->Get(), &rtvDesc, rtvHandle_);

    srvIndex_ = srv_->Allocate();
    srv_->CreateSRVforTexture2D(srvIndex_, renderTexture_->Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);
    srvHandle_ = srv_->GetGPUHandle(srvIndex_);

    Log::Send(Log::Level::INFO, "PostProcessExecutor scene render texture created successfully");
}
