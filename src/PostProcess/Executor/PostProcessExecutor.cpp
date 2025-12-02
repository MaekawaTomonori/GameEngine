#define NOMINMAX
#include "PostProcessExecutor.hpp"

#include "DebugUI.hpp"
#include "Log.hpp"
#include "imgui.h"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/Heap.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/PostProcess/IPostEffect.hpp"
#include "src/PostProcess/Editor/PostProcessPresetEditor.hpp"
#include "Factory/AbstractPostEffectFactory.hpp"
#include "vendor/json/json.hpp"
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

void PostProcessExecutor::Initialize(DirectXAdapter* _adapter, SRVManager* _srv, DebugUI* _debug) {
    adapter_ = _adapter;
    srv_ = _srv;
    debugUI_ = _debug;

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
        // エフェクト用のRTVハンドルを割り当て
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

    // エフェクトチェーン処理
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

    // PSO設定
    pso_->DrawCall();

//#ifdef _DEBUG

    //debugUI_->RegisterCommand("SceneRendering", [&]() {
    //    ImGui::Begin("Scene");
    //    
    //    // RenderTextureをImGuiで表示
    //    if (srvHandle_.ptr != 0) {
    //        ImTextureID textureID = srvHandle_.ptr;
    //        ImVec2 imageSize = ImVec2(static_cast<float>(adapter_->GetWidth()), static_cast<float>(adapter_->GetHeight()));
    //        ImGui::Image(textureID, imageSize);
    //    }
    //    
    //    ImGui::End();
    //});

//#else
    adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(0, srvHandle_);

    // フルスクリーンクワッドを三角形で描画
    adapter_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
//#endif
}

void PostProcessExecutor::SetActive(const std::string& _name, bool _enable) {
    for (auto& effect : effects_) {
        if (effect.name == _name) {
            effect.enabled = _enable;
            return;
        }
    }
}

void PostProcessExecutor::Debug() {
    static char presetNameBuf[128] = "MyPreset";
    static char loadPresetBuf[128] = "PresetName";

    debugUI_->RegisterCommand("PostEffect", [&](){
        ImGui::Begin("PostEffect");

        // Open Preset Editor button
        if (ImGui::Button("Open Preset Editor", ImVec2(-1, 30))) {
            OpenPresetEditor();
        }

        if (ImGui::BeginTabBar("PostEffect")){
            if (ImGui::BeginTabItem("List")) {
                for (auto& effect : effects_) {
                    ImGui::Checkbox(effect.name.c_str(), &effect.enabled);
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Details")){
                for (auto& effect : effects_) {
                    ImGui::PushID(effect.name.c_str());
                    if (ImGui::TreeNode(effect.name.c_str())){
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
    });

    // Show preset editor if open
    if (presetEditor_ && presetEditor_->IsOpen()) {
        presetEditor_->ShowEditor();
    }
}

void PostProcessExecutor::CreateSceneRenderTexture() {
    if (!adapter_) {
        return;
    }

    // RTVHeapを作成（シーン用1つ + エフェクト用複数）
    rtvHeap_ = std::make_unique<Heap>();
    if (!rtvHeap_->Create(adapter_->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 16, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)) {
        Log::Send(Log::Level::ERR, "Failed to create RTV heap for PostProcessExecutor");
        return;
    }

    // 画面サイズのRenderTextureを作成
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

void PostProcessExecutor::ResizeRenderTextures() {
    if (!adapter_) {
        return;
    }

    Log::Send(Log::Level::INFO,
        "PostProcessExecutor: Resizing render textures to " +
        std::to_string(adapter_->GetWidth()) + "x" + std::to_string(adapter_->GetHeight()));

    // 既存のレンダーテクスチャを解放
    if (renderTexture_) {
        renderTexture_.reset();
    }

    // 新しいサイズでレンダーテクスチャを再作成
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

    // RTVを再作成（既存のヒープに上書き）
    rtvHandle_ = rtvHeap_->GetCPUHandle(0);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    adapter_->GetDevice()->CreateRenderTargetView(renderTexture_->Get(), &rtvDesc, rtvHandle_);

    // SRVを再作成（同じインデックスに上書き）
    srv_->CreateSRVforTexture2D(srvIndex_, renderTexture_->Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);
    srvHandle_ = srv_->GetGPUHandle(srvIndex_);

    Log::Send(Log::Level::INFO, "PostProcessExecutor render textures resized successfully");
}

IPostEffect* PostProcessExecutor::FindOrCreate(const std::string& _type, const std::string& _name, bool _create) {
    // 既存インスタンス検索
    for (auto& effectData : effects_) {
        if (effectData.name == _name) {
            return effectData.effect.get();
        }
    }

    // createがfalseなら生成しない
    if (!_create) return nullptr;

    // Factoryが設定されていない場合
    if (!factory_) {
        Log::Send(Log::Level::ERR, "PostEffectFactory is not set");
        return nullptr;
    }

    // Factoryでエフェクト生成
    auto newEffect = factory_->Create(_type);
    if (!newEffect) {
        Log::Send(Log::Level::ERR, std::format("Failed to create effect type: {}", _type));
        return nullptr;
    }

    // エフェクトをセットアップ
    newEffect->SetUp(adapter_, srv_);
    auto rtvHandle = rtvHeap_->GetCPUHandle(static_cast<uint32_t>(effects_.size()) + 1);
    newEffect->SetRTVHandle(rtvHandle);
    newEffect->Initialize();

    effects_.emplace_back(EffectData{std::move(newEffect), _name, _type, true});

    return effects_.back().effect.get();
}

void PostProcessExecutor::ApplyPreset(const std::string& _presetName, const std::string& _mode, const std::vector<std::string>& _ignoreList, std::function<void()> _onComplete) {
    // コールバックを保存
    onAnimationComplete_ = _onComplete;

    std::string path = "./Assets/Data/PostEffect/presets.json";

    // ファイルが存在しない場合はエラー
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

    // プリセット名が存在するか確認
    if (!presetsJson.contains(_presetName)) {
        Log::Send(Log::Level::WARNING, std::format("Preset '{}' not found in presets.json", _presetName));
        return;
    }

    const auto& presetData = presetsJson[_presetName];

    // duration取得（Q38）
    animationDuration_ = presetData.value("duration", 0.0f);

    // Q35: replaceモードなら既存エフェクトを無効化
    if (_mode == "replace") {
        for (auto& effectData : effects_) {
            effectData.enabled = false;
        }
    }

    // アニメーション対象エフェクトリストをクリア
    animatingEffects_.clear();

    // effects配列またはmembers配列を処理（後方互換性のため両方対応）
    const char* arrayKey = presetData.contains("effects") ? "effects" : "members";

    if (presetData.contains(arrayKey)) {
        for (const auto& effectEntry : presetData[arrayKey]) {
            std::string type = effectEntry["type"];
            std::string name = effectEntry["name"];
            // presetキーがない場合はプリセット名と同じものを使用
            std::string presetName = effectEntry.value("preset", _presetName);
            bool autoCreate = effectEntry.value("autoCreate", true);

            // Q36: ignoreリストに含まれるエフェクトはスキップ
            bool shouldIgnore = false;
            for (const auto& ignoreName : _ignoreList) {
                if (ignoreName == name) {
                    shouldIgnore = true;
                    break;
                }
            }
            if (shouldIgnore) continue;

            // FindOrCreateで遅延初期化
            IPostEffect* effect = FindOrCreate(type, name, autoCreate);
            if (!effect) {
                Log::Send(Log::Level::WARNING, std::format("Failed to create effect: {} ({})", name, type));
                continue;
            }

            // プリセット読み込み
            effect->LoadPreset(presetName);
            Log::Send(Log::Level::DBG, std::format("Loaded preset '{}' for effect '{}' ({})", presetName, name, type));

            // エフェクトを有効化
            SetActive(name, true);
            Log::Send(Log::Level::DBG, std::format("Enabled effect '{}' (enabled: {})", name, true));

            // アニメーション対象リストに追加
            animatingEffects_.push_back(name);
        }
    }

    // アニメーション開始
    if (animationDuration_ > 0.0f) {
        isAnimating_ = true;
        animationTimer_ = 0.0f;
        Log::Send(Log::Level::INFO, std::format("Started animation for preset '{}' (duration: {:.1f}s, effects: {})",
            _presetName, animationDuration_, animatingEffects_.size()));
    } else {
        // Q42: duration=0なら即座に最終状態を適用
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

    // デバッグ: 全エフェクトの状態を出力
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

    // 全てのアニメーション対象エフェクトを更新
    for (const auto& effectName : animatingEffects_) {
        for (auto& effectData : effects_) {
            if (effectData.name == effectName) {
                effectData.effect->UpdateAnimation(t);
                break;
            }
        }
    }

    // アニメーション完了チェック
    if (t >= 1.0f) {
        isAnimating_ = false;

        // アニメーション完了後、該当エフェクトをデフォルト値に戻して無効化
        for (const auto& effectName : animatingEffects_) {
            for (auto& effectData : effects_) {
                if (effectData.name == effectName) {
                    // エフェクトを無効化
                    effectData.enabled = false;

                    // デフォルト値に戻すため、Initialize()を再実行
                    effectData.effect->Initialize();

                    Log::Send(Log::Level::INFO, std::format("Reset and disabled effect '{}'", effectName));
                    break;
                }
            }
        }

        // アニメーション対象リストをクリア
        animatingEffects_.clear();

        Log::Send(Log::Level::INFO, "PostEffect animation completed and effects reset");

        // コールバックが設定されている場合は呼び出す
        if (onAnimationComplete_) {
            Log::Send(Log::Level::INFO, "Calling animation complete callback");
            onAnimationComplete_();
            onAnimationComplete_ = nullptr;  // コールバックをクリア
        }
    }
}

void PostProcessExecutor::SavePreset(const std::string& _presetName) {
    json presetJson;

    // 有効なエフェクトのみ保存（Q44）
    for (const auto& effectData : effects_) {
        if (effectData.enabled) {
            json effectEntry;
            effectEntry["type"] = effectData.type;
            effectEntry["name"] = effectData.name;
            effectEntry["preset"] = _presetName;
            effectEntry["autoCreate"] = true;

            presetJson["effects"].push_back(effectEntry);

            // 各エフェクトのパラメータを保存
            effectData.effect->SavePreset(_presetName);
        }
    }

    // duration保存
    presetJson["duration"] = animationDuration_;

    // presets.jsonに保存
    std::string presetsPath = "./Assets/Data/PostEffect/presets.json";
    std::filesystem::create_directories("./Assets/Data/PostEffect");

    // 既存のpresets.jsonを読み込み
    json allPresets;
    if (std::filesystem::exists(presetsPath)) {
        std::ifstream inFile(presetsPath);
        if (inFile.is_open()) {
            inFile >> allPresets;
            inFile.close();
        }
    }

    // 新しいプリセットを追加
    allPresets[_presetName] = presetJson;

    // 保存
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
