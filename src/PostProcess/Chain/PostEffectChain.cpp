#include "PostEffectChain.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <utility>

#include "Log.hpp"
#include "Factory/PostEffectFactory.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/Heap.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/PostProcess/IPostEffect.hpp"

#undef min
#undef max

using json = nlohmann::json;

void PostEffectChain::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv, const std::string& _name) {
    adapter_ = _adapter;
    srv_ = _srv;
    name_ = _name;

    if (!adapter_) {
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return;
    }

    // シーン描画用の RenderTexture を作成する。
    CreateSceneRenderTexture();

    D3D12_DESCRIPTOR_RANGE range{
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
    };

    // ポストプロセス結果を画面へコピーするための PSO。
    pso_ = std::make_unique<PipelineStateObject>(adapter_);
    pso_->SetRootSignature(
        RootSignature().AddParameter({
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
    .SetBlend(BlendMode::PREMULTIPLIED)
    .SetShader(std::make_unique<Shader>(L"CpyImg"))
    .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
    .Create();
}

void PostEffectChain::SetFactory(GESTD::ReferencePtr<PostEffectFactory> _factory) {
    factory_ = _factory;
}

void PostEffectChain::Add(std::unique_ptr<IPostEffect> _effect) {
    if (!_effect) {
        Log::Send(Log::Level::ERR, "Attempted to add a null post effect");
        return;
    }

    _effect->SetUp(adapter_, srv_);

    const uint32_t slot = rtvHeap_->Allocate();
    _effect->SetRTVHandle(rtvHeap_->GetCPUHandle(slot));
    _effect->Initialize();

    const std::string type = _effect->GetTypeName();
    effects_.emplace_back(EffectData{ std::move(_effect), type, slot, true });
}

void PostEffectChain::BeginFrame() {
    ApplyPendingRemovals();

    if (!adapter_ || !renderTexture_) {
        Log::Send(Log::Level::ERR, "PostEffectChain is not properly initialized");
        return;
    }

    renderTexture_->ChangeState(adapter_->GetCommandList(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    auto dsvHandle = adapter_->GetDSVHandle();
    adapter_->GetCommandList()->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle);
    adapter_->GetCommandList()->ClearRenderTargetView(rtvHandle_, &clearColor_.x, 0, nullptr);

    adapter_->PreProcess();
}

void PostEffectChain::EndFrame() const {
    if (!adapter_ || !renderTexture_) {
        return;
    }

    renderTexture_->ChangeState(adapter_->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

D3D12_GPU_DESCRIPTOR_HANDLE PostEffectChain::Execute() {
    if (!adapter_) {
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return {};
    }

    auto handle = srv_->GetGPUHandle(srvIndex_);

    // 常時構成のエフェクトを追加順に適用する。
    for (const auto& effect : effects_) {
        if (!effect.enabled) continue;
        handle = effect.effect->Apply(handle);
    }

    srvHandle_ = handle;
    return srvHandle_;
}

void PostEffectChain::Draw() const {
    if (!adapter_ || !pso_) {
        Log::Send(Log::Level::ERR, "PostEffectChain is not properly initialized");
        return;
    }

    srv_->PreDraw();
    adapter_->PreProcess();
    adapter_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // フルスクリーン描画用 PSO を適用する。
    pso_->DrawCall();

    adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(0, srvHandle_);

    // フルスクリーントライアングルで結果テクスチャを描画する。
    adapter_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

void PostEffectChain::SetActive(const std::string& _type, bool _enable) {
    for (auto& effect : effects_) {
        if (effect.type == _type) {
            effect.enabled = _enable;
            return;
        }
    }
}

void PostEffectChain::CreateSceneRenderTexture() {
    if (!adapter_) {
        return;
    }

    // Scene 用 1 枚と effect 用の RTV を格納する heap を作成する。
    rtvHeap_ = std::make_unique<Heap>();
    if (!rtvHeap_->Create(adapter_->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 16, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)) {
        Log::Send(Log::Level::ERR, "Failed to create RTV heap for PostEffectChain");
        return;
    }

    rtvHandle_ = rtvHeap_->GetCPUHandle(rtvHeap_->Allocate());

    // Scene 描画先の RenderTexture を生成する。
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

    // Scene texture 用の SRV index を確保する。
    srvIndex_ = srv_->Allocate();

    // RTV/SRV を作成する。
    CreateRenderTextureViews();

    Log::Send(Log::Level::INFO, "PostEffectChain scene render texture created successfully");
}

void PostEffectChain::CreateRenderTextureViews() {
    if (!renderTexture_ || !renderTexture_->Get()) {
        Log::Send(Log::Level::ERR, "Cannot create views: render texture is null");
        return;
    }

    // RTV を作成する。
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    adapter_->GetDevice()->CreateRenderTargetView(renderTexture_->Get(), &rtvDesc, rtvHandle_);

    // SRV を作成する。
    srv_->CreateSRVForTexture2D(srvIndex_, renderTexture_->Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);
    srvHandle_ = srv_->GetGPUHandle(srvIndex_);
}

void PostEffectChain::ResizeRenderTextures() {
    if (!adapter_) {
        return;
    }

    Log::Send(Log::Level::INFO,
        "PostEffectChain: Resizing render textures to " +
        std::to_string(adapter_->GetWidth()) + "x" + std::to_string(adapter_->GetHeight()));

    // 旧 RenderTexture を破棄する。
    if (renderTexture_) {
        renderTexture_.reset();
    }

    // 新しいサイズで RenderTexture を再作成する。
    renderTexture_ = adapter_->CreateRenderTextureResource(
        static_cast<uint32_t>(adapter_->GetWidth()),
        static_cast<uint32_t>(adapter_->GetHeight()),
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        clearColor_
    );

    renderTexture_->Get()->SetName(L"RenderTexture");

    if (!renderTexture_->Get()) {
        Log::Send(Log::Level::ERR, "Failed to recreate scene render texture");
        return;
    }

    // RTV/SRV を再作成する。
    CreateRenderTextureViews();

    Log::Send(Log::Level::INFO, "PostEffectChain render textures resized successfully");
}

ID3D12Resource* PostEffectChain::GetRenderTextureResource() const {
    return renderTexture_ ? renderTexture_->Get() : nullptr;
}

ID3D12Resource* PostEffectChain::GetPreviewResource() const {
    for (auto it = effects_.rbegin(); it != effects_.rend(); ++it) {
        if (!it->enabled) continue;
        if (ID3D12Resource* output = it->effect->GetOutputResource()) {
            return output;
        }
    }
    return GetRenderTextureResource();
}

float PostEffectChain::GetAspectRatio() const {
    if (!renderTexture_ || !renderTexture_->Get()) return 1.0f;
    const D3D12_RESOURCE_DESC desc = renderTexture_->Get()->GetDesc();
    if (desc.Height == 0) return 1.0f;
    return static_cast<float>(desc.Width) / static_cast<float>(desc.Height);
}

uint32_t PostEffectChain::AllocateRtvSlot() const {
    return rtvHeap_->Allocate();
}

void PostEffectChain::FreeRtvSlot(uint32_t _slot) const {
    rtvHeap_->Free(_slot);
}

D3D12_CPU_DESCRIPTOR_HANDLE PostEffectChain::GetRtvCpuHandle(uint32_t _slot) const {
    return rtvHeap_->GetCPUHandle(_slot);
}

IPostEffect* PostEffectChain::Create(const std::string& _type) {
    // 既存 instance を検索する。
    for (auto& effectData : effects_) {
        if (effectData.type == _type) {
            return effectData.effect.get();
        }
    }

    // Factory が未設定なら生成できない。
    if (!factory_) {
        Log::Send(Log::Level::ERR, "PostEffectFactory is not set");
        return nullptr;
    }

    // Factory から新しい effect を生成する。
    auto newEffect = factory_->Create(_type);
    if (!newEffect) {
        Log::Send(Log::Level::ERR, std::format("Failed to create effect type: {}", _type));
        return nullptr;
    }

    // effect をチェーン配下へ組み込む。
    newEffect->SetUp(adapter_, srv_);
    const uint32_t slot = rtvHeap_->Allocate();
    newEffect->SetRTVHandle(rtvHeap_->GetCPUHandle(slot));
    newEffect->Initialize();

    effects_.emplace_back(EffectData{ std::move(newEffect), _type, slot, true });

    return effects_.back().effect.get();
}

void PostEffectChain::RemoveEffect(const std::string& _type) {
    pendingRemovals_.push_back(_type);
}

void PostEffectChain::ApplyPendingRemovals() {
    for (const auto& type : pendingRemovals_) {
        for (auto it = effects_.begin(); it != effects_.end(); ++it) {
            if (it->type == type) {
                rtvHeap_->Free(it->rtvSlot);

                // GPU Based Validationは共有SRVヒープ上のディスクリプタが指すリソースを
                // 常に使用中とみなすため、破棄前に生存中のrenderTexture_へ向け直す
                srv_->CreateSRVForTexture2D(it->effect->GetSrvIndex(), renderTexture_->Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);

                effects_.erase(it);
                break;
            }
        }
    }
    pendingRemovals_.clear();
}

void PostEffectChain::MoveEffectUp(const std::string& _type) {
    for (size_t i = 1; i < effects_.size(); ++i) {
        if (effects_[i].type == _type) {
            std::swap(effects_[i], effects_[i - 1]);
            return;
        }
    }
}

void PostEffectChain::MoveEffectDown(const std::string& _type) {
    for (size_t i = 0; i + 1 < effects_.size(); ++i) {
        if (effects_[i].type == _type) {
            std::swap(effects_[i], effects_[i + 1]);
            return;
        }
    }
}

void PostEffectChain::LoadPermanentConfig() {
    std::string path = PermanentConfigPath();
    if (!std::filesystem::exists(path)) return;

    std::ifstream file(path);
    if (!file.is_open()) {
        Log::Send(Log::Level::ERR, std::format("Failed to open canvas config: {}", path));
        return;
    }

    json configJson;
    file >> configJson;
    file.close();

    if (!configJson.contains("effects")) return;

    for (const auto& entry : configJson["effects"]) {
        std::string type = entry.value("type", "");
        if (type.empty()) continue;

        IPostEffect* effect = Create(type);
        if (!effect) continue;

        if (entry.contains("baseParameters")) {
            effect->ApplyParameters(entry["baseParameters"]);
        }
    }

    Log::Send(Log::Level::INFO, std::format("Loaded permanent config for '{}'", name_));
}

void PostEffectChain::SavePermanentConfig() const {
    json configJson;
    json effectsArray = json::array();

    for (const auto& effectData : effects_) {
        json entry;
        entry["type"] = effectData.type;
        entry["baseParameters"] = effectData.effect->CaptureCurrentParameters();
        effectsArray.push_back(entry);
    }

    configJson["effects"] = effectsArray;

    std::filesystem::create_directories("./Assets/Data/PostEffect/Canvases");

    std::ofstream file(PermanentConfigPath());
    if (!file.is_open()) {
        Log::Send(Log::Level::ERR, std::format("Failed to save canvas config for '{}'", name_));
        return;
    }

    file << configJson.dump(4);
    file.close();

    Log::Send(Log::Level::INFO, std::format("Saved permanent config for '{}'", name_));
}
