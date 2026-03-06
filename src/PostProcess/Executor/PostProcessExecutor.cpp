#include "PostProcessExecutor.hpp"

#include "imgui.h"
#include "json.hpp"
#include "DebugUI.hpp"
#include "Log.hpp"
#include "Input.hpp"
#include "Pattern/Singleton.hpp"
#include "Factory/AbstractPostEffectFactory.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/Heap.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/PostProcess/IPostEffect.hpp"
#include "src/PostProcess/Editor/PostProcessPresetEditor.hpp"
#include <fstream>
#include <filesystem>

#undef min
#undef max

using json = nlohmann::json;

void PostProcessExecutor::Initialize(DirectXAdapter* _adapter, SRVManager* _srv, DebugUI* _debug) {
    adapter_ = _adapter;
    srv_ = _srv;
    debugUI_ = _debug;

#ifdef _DEBUG
    if (debugUI_) {
        debugUI_->RegisterMenuButton("PostEffect");
        debugUI_->RegisterMenuButton("Scene");
    }
#endif

    if (!adapter_) {
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return;
    }

    // 繧ｷ繝ｼ繝ｳ逕ｨRenderTexture繧剃ｽ懈・
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
    .SetBlend(BlendMode::NONE)
    .SetShader(std::make_unique<Shader>(L"CpyImg"))
    .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
    .Create();

    // Initialize preset editor
    presetEditor_ = std::make_unique<PostProcessPresetEditor>();
    presetEditor_->Initialize(debugUI_, this);
}

void PostProcessExecutor::SetFactory(AbstractPostEffectFactory* _factory) {
    factory_ = _factory;
}

void PostProcessExecutor::Add(std::unique_ptr<IPostEffect> _effect, const std::string& _name) {
    if (_effect){
        _effect->SetUp(adapter_, srv_);
        // 繧ｨ繝輔ぉ繧ｯ繝育畑縺ｮRTV繝上Φ繝峨Ν繧貞牡繧雁ｽ薙※
        auto rtvHandle = rtvHeap_->GetCPUHandle(static_cast<uint32_t>(effects_.size()) + 1);
        _effect->SetRTVHandle(rtvHandle);
        _effect->Initialize();
        effects_.emplace_back(EffectData{std::move(_effect), _name, "", true});
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

void PostProcessExecutor::Execute() {
    if (!adapter_) {
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return;
    }

    auto handle = srv_->GetGPUHandle(srvIndex_);

    // 繧ｨ繝輔ぉ繧ｯ繝医メ繧ｧ繝ｼ繝ｳ蜃ｦ逅・
    for (const auto& effect : effects_) {
        if (!effect.enabled) continue;
        handle = effect.effect->Apply(handle);
    }

    srvHandle_ = handle;
}

void PostProcessExecutor::Draw() const {
    if (!adapter_ || !pso_) {
        Log::Send(Log::Level::ERR, "PostProcessExecutor is not properly initialized");
        return;
    }

    srv_->PreDraw();
    adapter_->PreProcess();
    adapter_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // PSO險ｭ螳・
    pso_->DrawCall();

    adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(0, srvHandle_);

    // 繝輔Ν繧ｹ繧ｯ繝ｪ繝ｼ繝ｳ繧ｯ繝ｯ繝・ラ繧剃ｸ芽ｧ貞ｽ｢縺ｧ謠冗判
    adapter_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

void PostProcessExecutor::SetActive(const std::string& _name, bool _enable) {
    for (auto& effect : effects_) {
        if (effect.name == _name) {
            effect.enabled = _enable;
            return;
        }
    }
}

bool PostProcessExecutor::IsSceneViewActive() {
    if (!debugUI_) return false;
    return debugUI_->IsVisible("Scene");
}

void PostProcessExecutor::Debug() {
#ifdef _DEBUG
    // Scene 縺碁撼陦ｨ遉ｺ縺ｮ蝣ｴ蜷医√・繧ｦ繧ｹ蠎ｧ讓吝､画鋤繧偵Μ繧ｻ繝・ヨ
    if (!debugUI_->IsVisible("Scene")) {
        Singleton<Input>::GetInstance()->SetSceneViewTransform(false, {}, {}, {});
    }

        debugUI_->RegisterCommand("Scene", [this]() {
        bool& visible = debugUI_->IsVisible("Scene");
        const bool beginResult = ImGui::Begin("Scene", &visible);
        bool hasSceneImage = false;
        ImVec2 imgMin{};
        ImVec2 imgSize{};

        if (beginResult && sceneImGuiTextureId_ != 0 && adapter_) {
            const ImVec2 avail = ImGui::GetContentRegionAvail();
            const float renderW = static_cast<float>(adapter_->GetWidth());
            const float renderH = static_cast<float>(adapter_->GetHeight());
            const float scaleX = avail.x / renderW;
            const float scaleY = (avail.y > 0.f) ? (avail.y / renderH) : scaleX;
            const float scale  = std::min(scaleX, scaleY);
            sceneViewScale_ = scale;
            const float displayW = renderW * scale;
            const float displayH = renderH * scale;
            ImGui::Image(static_cast<ImTextureID>(sceneImGuiTextureId_), ImVec2(displayW, displayH));

            imgMin = ImGui::GetItemRectMin();
            imgSize = ImGui::GetItemRectSize();
            hasSceneImage = (imgSize.x > 0.f && imgSize.y > 0.f);
        }

        ImGui::End();

        if (hasSceneImage) {
            Singleton<Input>::GetInstance()->SetSceneViewTransform(
                true,
                { imgMin.x,  imgMin.y  },
                { imgSize.x, imgSize.y },
                { static_cast<float>(adapter_->GetWidth()), static_cast<float>(adapter_->GetHeight()) }
            );
        } else {
            Singleton<Input>::GetInstance()->SetSceneViewTransform(false, {}, {}, {});
        }
    });
#endif

    debugUI_->RegisterCommand("PostEffect", [this](){
        bool& visible = debugUI_->IsVisible("PostEffect");
        ImGui::Begin("PostEffect", &visible);

        if (ImGui::Button("Open Preset Editor", ImVec2(-1, 30))) {
            OpenPresetEditor();
        }

        if (ImGui::BeginTabBar("##PostEffectTabs")) {
            if (ImGui::BeginTabItem("List")) {
                for (auto& effect : effects_) {
                    ImGui::Checkbox(effect.name.c_str(), &effect.enabled);
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Details")) {
                for (auto& effect : effects_) {
                    ImGui::PushID(effect.name.c_str());
                    if (ImGui::TreeNode(effect.name.c_str())) {
                        effect.effect->Debug();
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        // PostEffect 繧ｦ繧｣繝ｳ繝峨え繧帝哩縺倥◆繧・PresetEditor 繧る｣蜍輔＠縺ｦ髢峨§繧・
        if (!visible && presetEditor_) {
            presetEditor_->CloseEditor();
        }
    });

    if (presetEditor_ && presetEditor_->IsOpen()) {
        presetEditor_->ShowEditor();
    }
}

void PostProcessExecutor::CreateSceneRenderTexture() {
    if (!adapter_) {
        return;
    }

    // RTVHeap繧剃ｽ懈・・医す繝ｼ繝ｳ逕ｨ1縺､ + 繧ｨ繝輔ぉ繧ｯ繝育畑隍・焚・・
    rtvHeap_ = std::make_unique<Heap>();
    if (!rtvHeap_->Create(adapter_->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 16, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)) {
        Log::Send(Log::Level::ERR, "Failed to create RTV heap for PostProcessExecutor");
        return;
    }

    // 逕ｻ髱｢繧ｵ繧､繧ｺ縺ｮRenderTexture繧剃ｽ懈・
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

    // SRV繧､繝ｳ繝・ャ繧ｯ繧ｹ繧貞牡繧雁ｽ薙※・亥・蝗槭・縺ｿ・・
    srvIndex_ = srv_->Allocate();

    // RTV/SRV險倩ｿｰ蟄舌ｒ菴懈・・亥・騾壼喧繝｡繧ｽ繝・ラ蜻ｼ縺ｳ蜃ｺ縺暦ｼ・
    CreateRenderTextureViews();

    // ImGui 繧ｷ繝ｼ繝ｳ繝薙Η繝ｼ逕ｨ縺ｫ DebugUI 縺ｮ繝偵・繝励∈ SRV 繧堤匳骭ｲ
    if (debugUI_) {
        sceneImGuiTextureId_ = debugUI_->RegisterTexture(renderTexture_->Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    }

    Log::Send(Log::Level::INFO, "PostProcessExecutor scene render texture created successfully");
}

void PostProcessExecutor::CreateRenderTextureViews() {
    if (!renderTexture_ || !renderTexture_->Get()) {
        Log::Send(Log::Level::ERR, "Cannot create views: render texture is null");
        return;
    }

    // RTV繧剃ｽ懈・
    rtvHandle_ = rtvHeap_->GetCPUHandle(0);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    adapter_->GetDevice()->CreateRenderTargetView(renderTexture_->Get(), &rtvDesc, rtvHandle_);

    // SRV繧剃ｽ懈・・医∪縺溘・譖ｴ譁ｰ・・
    srv_->CreateSRVForTexture2D(srvIndex_, renderTexture_->Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);
    srvHandle_ = srv_->GetGPUHandle(srvIndex_);
}

void PostProcessExecutor::ResizeRenderTextures() {
    if (!adapter_) {
        return;
    }

    Log::Send(Log::Level::INFO,
        "PostProcessExecutor: Resizing render textures to " +
        std::to_string(adapter_->GetWidth()) + "x" + std::to_string(adapter_->GetHeight()));

    // 譌｢蟄倥・繝ｬ繝ｳ繝繝ｼ繝・け繧ｹ繝√Ε繧定ｧ｣謾ｾ
    if (renderTexture_) {
        renderTexture_.reset();
    }

    // 譁ｰ縺励＞繧ｵ繧､繧ｺ縺ｧ繝ｬ繝ｳ繝繝ｼ繝・け繧ｹ繝√Ε繧貞・菴懈・
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

    // RTV/SRV險倩ｿｰ蟄舌ｒ蜀堺ｽ懈・・亥・騾壼喧繝｡繧ｽ繝・ラ蜻ｼ縺ｳ蜃ｺ縺暦ｼ・
    CreateRenderTextureViews();

    // ImGui 繧ｷ繝ｼ繝ｳ繝薙Η繝ｼ逕ｨ繝・け繧ｹ繝√Ε繧呈眠縺励＞繝ｪ繧ｽ繝ｼ繧ｹ縺ｧ譖ｴ譁ｰ
    if (debugUI_) {
        sceneImGuiTextureId_ = debugUI_->RegisterTexture(renderTexture_->Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    }

    Log::Send(Log::Level::INFO, "PostProcessExecutor render textures resized successfully");
}

IPostEffect* PostProcessExecutor::FindOrCreate(const std::string& _type, const std::string& _name, bool _create) {
    // 譌｢蟄倥う繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ讀懃ｴ｢
    for (auto& effectData : effects_) {
        if (effectData.name == _name) {
            return effectData.effect.get();
        }
    }

    // create縺掲alse縺ｪ繧臥函謌舌＠縺ｪ縺・
    if (!_create) return nullptr;

    // Factory縺瑚ｨｭ螳壹＆繧後※縺・↑縺・ｴ蜷・
    if (!factory_) {
        Log::Send(Log::Level::ERR, "PostEffectFactory is not set");
        return nullptr;
    }

    // Factory縺ｧ繧ｨ繝輔ぉ繧ｯ繝育函謌・
    auto newEffect = factory_->Create(_type);
    if (!newEffect) {
        Log::Send(Log::Level::ERR, std::format("Failed to create effect type: {}", _type));
        return nullptr;
    }

    // 繧ｨ繝輔ぉ繧ｯ繝医ｒ繧ｻ繝・ヨ繧｢繝・・
    newEffect->SetUp(adapter_, srv_);
    auto rtvHandle = rtvHeap_->GetCPUHandle(static_cast<uint32_t>(effects_.size()) + 1);
    newEffect->SetRTVHandle(rtvHandle);
    newEffect->Initialize();

    effects_.emplace_back(EffectData{std::move(newEffect), _name, _type, true});

    return effects_.back().effect.get();
}

void PostProcessExecutor::ApplyPreset(const std::string& _presetName, const std::string& _mode, const std::vector<std::string>& _ignoreList, std::function<void()> _onComplete) {
    // 繧ｳ繝ｼ繝ｫ繝舌ャ繧ｯ繧剃ｿ晏ｭ・
    onAnimationComplete_ = _onComplete;

    std::string path = "./Assets/Data/PostEffect/presets.json";

    // 繝輔ぃ繧､繝ｫ縺悟ｭ伜惠縺励↑縺・ｴ蜷医・繧ｨ繝ｩ繝ｼ
    if (!std::filesystem::exists(path)) {
        Log::Send(Log::Level::ERR, std::format("Preset file not found: {}", path));
        return;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        Log::Send(Log::Level::ERR, std::format("Failed to open preset file: {}", path));
        return;
    }

    json presetsJson;
    file >> presetsJson;
    file.close();

    // 繝励Μ繧ｻ繝・ヨ蜷阪′蟄伜惠縺吶ｋ縺狗｢ｺ隱・
    if (!presetsJson.contains(_presetName)) {
        Log::Send(Log::Level::WARNING, std::format("Preset '{}' not found in presets.json", _presetName));
        return;
    }

    const auto& presetData = presetsJson[_presetName];

    // duration蜿門ｾ暦ｼ・38・・
    animationDuration_ = presetData.value("duration", 0.0f);

    // Q35: replace繝｢繝ｼ繝峨↑繧画里蟄倥お繝輔ぉ繧ｯ繝医ｒ辟｡蜉ｹ蛹・
    if (_mode == "replace") {
        for (auto& effectData : effects_) {
            effectData.enabled = false;
        }
    }

    // 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ蟇ｾ雎｡繧ｨ繝輔ぉ繧ｯ繝医Μ繧ｹ繝医ｒ繧ｯ繝ｪ繧｢
    animatingEffects_.clear();

    // effects驟榊・縺ｾ縺溘・members驟榊・繧貞・逅・ｼ亥ｾ梧婿莠呈鋤諤ｧ縺ｮ縺溘ａ荳｡譁ｹ蟇ｾ蠢懶ｼ・
    const char* arrayKey = presetData.contains("effects") ? "effects" : "members";

    if (presetData.contains(arrayKey)) {
        for (const auto& effectEntry : presetData[arrayKey]) {
            std::string type = effectEntry["type"];
            std::string name = effectEntry["name"];
            // preset繧ｭ繝ｼ縺後↑縺・ｴ蜷医・繝励Μ繧ｻ繝・ヨ蜷阪→蜷後§繧ゅ・繧剃ｽｿ逕ｨ
            std::string presetName = effectEntry.value("preset", _presetName);
            bool autoCreate = effectEntry.value("autoCreate", true);

            // Q36: ignore繝ｪ繧ｹ繝医↓蜷ｫ縺ｾ繧後ｋ繧ｨ繝輔ぉ繧ｯ繝医・繧ｹ繧ｭ繝・・
            bool shouldIgnore = false;
            for (const auto& ignoreName : _ignoreList) {
                if (ignoreName == name) {
                    shouldIgnore = true;
                    break;
                }
            }
            if (shouldIgnore) continue;

            // FindOrCreate縺ｧ驕・ｻｶ蛻晄悄蛹・
            IPostEffect* effect = FindOrCreate(type, name, autoCreate);
            if (!effect) {
                Log::Send(Log::Level::WARNING, std::format("Failed to create effect: {} ({})", name, type));
                continue;
            }

            // 繝励Μ繧ｻ繝・ヨ隱ｭ縺ｿ霎ｼ縺ｿ
            effect->LoadPreset(presetName);
            Log::Send(Log::Level::DBG, std::format("Loaded preset '{}' for effect '{}' ({})", presetName, name, type));

            // 繧ｨ繝輔ぉ繧ｯ繝医ｒ譛牙柑蛹・
            SetActive(name, true);
            Log::Send(Log::Level::DBG, std::format("Enabled effect '{}' (enabled: {})", name, true));

            // 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ蟇ｾ雎｡繝ｪ繧ｹ繝医↓霑ｽ蜉
            animatingEffects_.push_back(name);
        }
    }

    // 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ髢句ｧ・
    if (animationDuration_ > 0.0f) {
        isAnimating_ = true;
        animationTimer_ = 0.0f;
        Log::Send(Log::Level::INFO, std::format("Started animation for preset '{}' (duration: {:.1f}s, effects: {})",
            _presetName, animationDuration_, animatingEffects_.size()));
    } else {
        // Q42: duration=0縺ｪ繧牙叉蠎ｧ縺ｫ譛邨ら憾諷九ｒ驕ｩ逕ｨ
        for (const auto& effectName : animatingEffects_) {
            for (auto& effectData : effects_) {
                if (effectData.name == effectName) {
                    effectData.effect->UpdateAnimation(1.0f);
                    break;
                }
            }
        }
        isAnimating_ = false;
    }

    // 繝・ヰ繝・げ: 蜈ｨ繧ｨ繝輔ぉ繧ｯ繝医・迥ｶ諷九ｒ蜃ｺ蜉・
    Log::Send(Log::Level::INFO, std::format("Total effects: {}", effects_.size()));
    for (const auto& effectData : effects_) {
        Log::Send(Log::Level::INFO, std::format("  Effect '{}' ({}): enabled={}",
            effectData.name, effectData.type, effectData.enabled ? "true" : "false"));
    }

    Log::Send(Log::Level::INFO, std::format("Preset '{}' applied with mode '{}'", _presetName, _mode));
}

void PostProcessExecutor::Update(float _deltaTime) {
    if (!isAnimating_) return;

    animationTimer_ += _deltaTime;
    float t = std::min(animationTimer_ / animationDuration_, 1.0f);

    // 蜈ｨ縺ｦ縺ｮ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ蟇ｾ雎｡繧ｨ繝輔ぉ繧ｯ繝医ｒ譖ｴ譁ｰ
    for (const auto& effectName : animatingEffects_) {
        for (auto& effectData : effects_) {
            if (effectData.name == effectName) {
                effectData.effect->UpdateAnimation(t);
                break;
            }
        }
    }

    // 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ螳御ｺ・メ繧ｧ繝・け
    if (t >= 1.0f) {
        isAnimating_ = false;

        // 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ螳御ｺ・ｾ後∬ｩｲ蠖薙お繝輔ぉ繧ｯ繝医ｒ繝・ヵ繧ｩ繝ｫ繝亥､縺ｫ謌ｻ縺励※辟｡蜉ｹ蛹・
        for (const auto& effectName : animatingEffects_) {
            for (auto& effectData : effects_) {
                if (effectData.name == effectName) {
                    // 繧ｨ繝輔ぉ繧ｯ繝医ｒ辟｡蜉ｹ蛹・
                    effectData.enabled = false;

                    // 繝・ヵ繧ｩ繝ｫ繝亥､縺ｫ謌ｻ縺吶◆繧√！nitialize()繧貞・螳溯｡・
                    effectData.effect->Initialize();

                    Log::Send(Log::Level::INFO, std::format("Reset and disabled effect '{}'", effectName));
                    break;
                }
            }
        }

        // 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ蟇ｾ雎｡繝ｪ繧ｹ繝医ｒ繧ｯ繝ｪ繧｢
        animatingEffects_.clear();

        Log::Send(Log::Level::INFO, "PostEffect animation completed and effects reset");

        // 繧ｳ繝ｼ繝ｫ繝舌ャ繧ｯ縺瑚ｨｭ螳壹＆繧後※縺・ｋ蝣ｴ蜷医・蜻ｼ縺ｳ蜃ｺ縺・
        if (onAnimationComplete_) {
            Log::Send(Log::Level::INFO, "Calling animation complete callback");
            onAnimationComplete_();
            onAnimationComplete_ = nullptr;  // 繧ｳ繝ｼ繝ｫ繝舌ャ繧ｯ繧偵け繝ｪ繧｢
        }
    }
}

void PostProcessExecutor::SavePreset(const std::string& _presetName) {
    json presetJson;

    // 譛牙柑縺ｪ繧ｨ繝輔ぉ繧ｯ繝医・縺ｿ菫晏ｭ假ｼ・44・・
    for (const auto& effectData : effects_) {
        if (effectData.enabled) {
            json effectEntry;
            effectEntry["type"] = effectData.type;
            effectEntry["name"] = effectData.name;
            effectEntry["preset"] = _presetName;
            effectEntry["autoCreate"] = true;

            presetJson["effects"].push_back(effectEntry);

            // 蜷・お繝輔ぉ繧ｯ繝医・繝代Λ繝｡繝ｼ繧ｿ繧剃ｿ晏ｭ・
            effectData.effect->SavePreset(_presetName);
        }
    }

    // duration菫晏ｭ・
    presetJson["duration"] = animationDuration_;

    // presets.json縺ｫ菫晏ｭ・
    std::string presetsPath = "./Assets/Data/PostEffect/presets.json";
    std::filesystem::create_directories("./Assets/Data/PostEffect");

    // 譌｢蟄倥・presets.json繧定ｪｭ縺ｿ霎ｼ縺ｿ
    json allPresets;
    if (std::filesystem::exists(presetsPath)) {
        std::ifstream inFile(presetsPath);
        if (inFile.is_open()) {
            inFile >> allPresets;
            inFile.close();
        }
    }

    // 譁ｰ縺励＞繝励Μ繧ｻ繝・ヨ繧定ｿｽ蜉
    allPresets[_presetName] = presetJson;

    // 菫晏ｭ・
    std::ofstream outFile(presetsPath);
    if (!outFile.is_open()) {
        Log::Send(Log::Level::ERR, std::format("Failed to save preset to: {}", presetsPath));
        return;
    }

    outFile << allPresets.dump(4);
    outFile.close();

    Log::Send(Log::Level::INFO, std::format("Preset '{}' saved successfully", _presetName));
}

void PostProcessExecutor::OpenPresetEditor(const std::string& _presetName) {
    if (presetEditor_) {
        presetEditor_->OpenEditor(_presetName);
    }
}
