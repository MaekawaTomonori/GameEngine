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

void PostProcessExecutor::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv, const GESTD::ReferencePtr<DebugUI>& _debug) {
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
    .SetBlend(BlendMode::NONE)
    .SetShader(std::make_unique<Shader>(L"CpyImg"))
    .SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
    .Create();

    // Preset editor を関連付ける。
    presetEditor_ = std::make_unique<PostProcessPresetEditor>();
    presetEditor_->Initialize(debugUI_, GESTD::ReferencePtr<PostProcessExecutor>(this));
}

void PostProcessExecutor::SetFactory(GESTD::ReferencePtr<AbstractPostEffectFactory> _factory) {
    factory_ = _factory;
}

void PostProcessExecutor::Add(std::unique_ptr<IPostEffect> _effect, const std::string& _name) {
    if (_effect) {
        _effect->SetUp(adapter_, srv_);

        // 各エフェクトに対応する RTV ハンドルを順番に割り当てる。
        auto rtvHandle = rtvHeap_->GetCPUHandle(static_cast<uint32_t>(effects_.size()) + 1);
        _effect->SetRTVHandle(rtvHandle);
        _effect->Initialize();
        effects_.emplace_back(EffectData{ std::move(_effect), _name, "", true });
    } else {
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

    // 有効なエフェクトを追加順に適用する。
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

    // フルスクリーン描画用 PSO を適用する。
    pso_->DrawCall();

    adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(0, srvHandle_);

    // フルスクリーントライアングルで結果テクスチャを描画する。
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
    // SceneView が閉じているときは入力座標変換を無効化する。
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
            const float scale = std::min(scaleX, scaleY);
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
                { imgMin.x, imgMin.y },
                { imgSize.x, imgSize.y },
                { static_cast<float>(adapter_->GetWidth()), static_cast<float>(adapter_->GetHeight()) }
            );
        } else {
            Singleton<Input>::GetInstance()->SetSceneViewTransform(false, {}, {}, {});
        }
    });
#endif

    debugUI_->RegisterCommand("PostEffect", [this]() {
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

        // PostEffect ウィンドウを閉じたら PresetEditor も閉じる。
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

    // Scene 用 1 枚と effect 用の RTV を格納する heap を作成する。
    rtvHeap_ = std::make_unique<Heap>();
    if (!rtvHeap_->Create(adapter_->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 16, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)) {
        Log::Send(Log::Level::ERR, "Failed to create RTV heap for PostProcessExecutor");
        return;
    }

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

    // DebugUI で SceneView 表示できるようにテクスチャを登録する。
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

    // RTV を作成する。
    rtvHandle_ = rtvHeap_->GetCPUHandle(0);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    adapter_->GetDevice()->CreateRenderTargetView(renderTexture_->Get(), &rtvDesc, rtvHandle_);

    // SRV を作成する。
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

    // DebugUI へ再登録する。
    if (debugUI_) {
        sceneImGuiTextureId_ = debugUI_->RegisterTexture(renderTexture_->Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    }

    Log::Send(Log::Level::INFO, "PostProcessExecutor render textures resized successfully");
}

IPostEffect* PostProcessExecutor::FindOrCreate(const std::string& _type, const std::string& _name, bool _create) {
    // 既存 instance を検索する。
    for (auto& effectData : effects_) {
        if (effectData.name == _name) {
            return effectData.effect.get();
        }
    }

    // 自動生成しない場合はここで終了。
    if (!_create) return nullptr;

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

    // effect を executor 配下へ組み込む。
    newEffect->SetUp(adapter_, srv_);
    auto rtvHandle = rtvHeap_->GetCPUHandle(static_cast<uint32_t>(effects_.size()) + 1);
    newEffect->SetRTVHandle(rtvHandle);
    newEffect->Initialize();

    effects_.emplace_back(EffectData{ std::move(newEffect), _name, _type, true });

    return effects_.back().effect.get();
}

void PostProcessExecutor::ApplyPreset(const std::string& _presetName, const std::string& _mode, const std::vector<std::string>& _ignoreList, std::function<void()> _onComplete) {
    // 完了時コールバックを保持する。
    onAnimationComplete_ = _onComplete;

    std::string path = "./Assets/Data/PostEffect/presets.json";

    // preset 定義ファイルの存在確認。
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

    // 指定 preset が存在するか確認する。
    if (!presetsJson.contains(_presetName)) {
        Log::Send(Log::Level::WARNING, std::format("Preset '{}' not found in presets.json", _presetName));
        return;
    }

    const auto& presetData = presetsJson[_presetName];

    // アニメーション時間を読み取る。
    animationDuration_ = presetData.value("duration", 0.0f);

    // replace モードでは既存 effect を一旦無効化する。
    if (_mode == "replace") {
        for (auto& effectData : effects_) {
            effectData.enabled = false;
        }
    }

    // 今回のアニメーション対象を初期化する。
    animatingEffects_.clear();

    // effects / members のどちらにも対応する。
    const char* arrayKey = presetData.contains("effects") ? "effects" : "members";

    if (presetData.contains(arrayKey)) {
        for (const auto& effectEntry : presetData[arrayKey]) {
            std::string type = effectEntry["type"];
            std::string name = effectEntry["name"];

            // preset 名省略時は現在の preset 名を使う。
            std::string presetName = effectEntry.value("preset", _presetName);
            bool autoCreate = effectEntry.value("autoCreate", true);

            // ignore list に含まれる effect はスキップする。
            bool shouldIgnore = false;
            for (const auto& ignoreName : _ignoreList) {
                if (ignoreName == name) {
                    shouldIgnore = true;
                    break;
                }
            }
            if (shouldIgnore) continue;

            // effect を取得または生成する。
            IPostEffect* effect = FindOrCreate(type, name, autoCreate);
            if (!effect) {
                Log::Send(Log::Level::WARNING, std::format("Failed to create effect: {} ({})", name, type));
                continue;
            }

            // preset を読み込み、有効化する。
            effect->LoadPreset(presetName);
            Log::Send(Log::Level::DBG, std::format("Loaded preset '{}' for effect '{}' ({})", presetName, name, type));

            SetActive(name, true);
            Log::Send(Log::Level::DBG, std::format("Enabled effect '{}' (enabled: {})", name, true));

            // アニメーション対象として記録する。
            animatingEffects_.push_back(name);
        }
    }

    // アニメーションありの場合は更新ループへ移行する。
    if (animationDuration_ > 0.0f) {
        isAnimating_ = true;
        animationTimer_ = 0.0f;
        Log::Send(Log::Level::INFO, std::format("Started animation for preset '{}' (duration: {:.1f}s, effects: {})",
            _presetName, animationDuration_, animatingEffects_.size()));
    } else {
        // duration が 0 の場合は即座に最終状態を適用する。
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

    // デバッグ出力。
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

    // アニメーション対象 effect を進行度 t で更新する。
    for (const auto& effectName : animatingEffects_) {
        for (auto& effectData : effects_) {
            if (effectData.name == effectName) {
                effectData.effect->UpdateAnimation(t);
                break;
            }
        }
    }

    // 最後まで到達したら状態を片付ける。
    if (t >= 1.0f) {
        isAnimating_ = false;

        // 一時的に使った effect を初期状態へ戻して無効化する。
        for (const auto& effectName : animatingEffects_) {
            for (auto& effectData : effects_) {
                if (effectData.name == effectName) {
                    effectData.enabled = false;
                    effectData.effect->Initialize();

                    Log::Send(Log::Level::INFO, std::format("Reset and disabled effect '{}'", effectName));
                    break;
                }
            }
        }

        // 対象一覧をクリアする。
        animatingEffects_.clear();

        Log::Send(Log::Level::INFO, "PostEffect animation completed and effects reset");

        // 完了コールバックがあれば呼び出す。
        if (onAnimationComplete_) {
            Log::Send(Log::Level::INFO, "Calling animation complete callback");
            onAnimationComplete_();
            onAnimationComplete_ = nullptr;
        }
    }
}

void PostProcessExecutor::SavePreset(const std::string& _presetName) {
    json presetJson;

    // 現在有効な effect だけを書き出す。
    for (const auto& effectData : effects_) {
        if (effectData.enabled) {
            json effectEntry;
            effectEntry["type"] = effectData.type;
            effectEntry["name"] = effectData.name;
            effectEntry["preset"] = _presetName;
            effectEntry["autoCreate"] = true;

            presetJson["effects"].push_back(effectEntry);

            // 各 effect のパラメータも保存する。
            effectData.effect->SavePreset(_presetName);
        }
    }

    // duration を保存する。
    presetJson["duration"] = animationDuration_;

    // presets.json を更新する。
    std::string presetsPath = "./Assets/Data/PostEffect/presets.json";
    std::filesystem::create_directories("./Assets/Data/PostEffect");

    // 既存ファイルがあれば読み込む。
    json allPresets;
    if (std::filesystem::exists(presetsPath)) {
        std::ifstream inFile(presetsPath);
        if (inFile.is_open()) {
            inFile >> allPresets;
            inFile.close();
        }
    }

    // 新しい preset を上書きまたは追加する。
    allPresets[_presetName] = presetJson;

    // ファイルへ書き戻す。
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
