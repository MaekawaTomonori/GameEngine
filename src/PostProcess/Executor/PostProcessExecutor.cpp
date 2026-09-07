#include "PostProcessExecutor.hpp"

#include "imgui.h"
#include "json.hpp"
#include "DebugUI.hpp"
#include "Log.hpp"
#include "Input.hpp"
#include "Pattern/Singleton.hpp"
#include "Factory/PostEffectFactory.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/PostProcess/IPostEffect.hpp"
#include "src/PostProcess/Editor/PostProcessPresetEditor.hpp"
#include <fstream>
#include <filesystem>

#undef min
#undef max

using json = nlohmann::json;

void PostProcessExecutor::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv, const GESTD::ReferencePtr<DebugUI>& _debug, const std::string& _name) {
    adapter_ = _adapter;
    srv_ = _srv;
    debugUI_ = _debug;
    name_ = _name;

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

    chain_ = std::make_unique<PostEffectChain>();
    chain_->Initialize(adapter_, srv_, name_);

    layer_.Initialize(adapter_, srv_);

    noneCanvas_ = std::make_unique<Canvas>();
    noneCanvas_->Initialize(adapter_, srv_, "None");

#ifdef _DEBUG
    // DebugUI の Scene プレビュー用に、最終合成結果をもう一度描いておくための専用チェーンを用意する。
    sceneCaptureChain_ = std::make_unique<PostEffectChain>();
    sceneCaptureChain_->SetClearColor(adapter_->GetBackgroundColor());
    sceneCaptureChain_->Initialize(adapter_, srv_, name_ + "SceneCapture");

    if (debugUI_) {
        sceneImGuiTextureId_ = debugUI_->RegisterTexture(sceneCaptureChain_->GetRenderTextureResource(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    }
#endif

    // Preset editor を関連付ける。
    presetEditor_ = std::make_unique<PostProcessPresetEditor>();
    presetEditor_->Initialize(debugUI_, GESTD::ReferencePtr<PostProcessExecutor>(this));

    // Canvas Editor を関連付ける。
    canvasLayerEditor_ = std::make_unique<CanvasLayerEditor>();
    canvasLayerEditor_->Initialize(debugUI_, GESTD::ReferencePtr<PostProcessExecutor>(this));
}

void PostProcessExecutor::SetFactory(GESTD::ReferencePtr<PostEffectFactory> _factory) {
    factory_ = _factory;
    chain_->SetFactory(_factory);
    layer_.SetFactory(_factory);
}

void PostProcessExecutor::Add(std::unique_ptr<IPostEffect> _effect) {
    chain_->Add(std::move(_effect));
}

Canvas* PostProcessExecutor::AddCanvas(const std::string& _name) {
    return layer_.AddCanvas(_name);
}

Canvas* PostProcessExecutor::AddCanvasTop(const std::string& _name) {
    return layer_.AddCanvasTop(_name);
}

void PostProcessExecutor::RemoveCanvas(const std::string& _name) {
    layer_.RemoveCanvas(_name);
}

Canvas* PostProcessExecutor::GetCanvas(const std::string& _name) const {
    if (_name == "None") return noneCanvas_.get();
    return layer_.GetCanvas(_name);
}

std::vector<std::pair<CanvasLayer::ZOrder, Canvas*>> PostProcessExecutor::GetCanvases() const {
    return layer_.GetCanvases();
}

void PostProcessExecutor::DrawObjects() {
    layer_.DrawObjects();
    noneCanvas_->DrawObjects();
}

void PostProcessExecutor::ApplyPostEffects() {
    layer_.ApplyPostEffects();
    noneCanvas_->ApplyPostEffects();
}

void PostProcessExecutor::DrawCanvases() {
    // 各Canvasを、このExecutor自身の合成用RTへ描画する。
    chain_->BeginFrame();
    layer_.DrawCanvases();
    chain_->EndFrame();

    // 自分専用の常時構成エフェクトを適用する（通常は空）。
    auto handle = chain_->Execute();

    // 合成後の画面に対してワンショット演出を適用する。
    for (const auto& temporary : temporaries_) {
        if (!temporary.enabled) continue;
        handle = temporary.effect->Apply(handle);
    }

    chain_->SetSrvHandle(handle);
}

void PostProcessExecutor::Draw() const {
#ifdef _DEBUG
    // Sceneプレビュー用に、最終合成結果を専用テクスチャへ先に描いておく。
    sceneCaptureChain_->BeginFrame();
    chain_->Draw();
    noneCanvas_->Draw();
    sceneCaptureChain_->EndFrame();

    // SwapChainのバインドを取り戻す（クリアされるが、まだ何も描いていないため問題ない）。
    adapter_->SetSwapChainRenderTarget();
#endif

    chain_->Draw();
    noneCanvas_->Draw();
}

void PostProcessExecutor::SetActive(const std::string& _type, bool _enable) {
    chain_->SetActive(_type, _enable);
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
        if (ImGui::Button("Open Canvas Editor", ImVec2(-1, 30))) {
            OpenCanvasEditor();
        }

        ImGui::Separator();
        ImGui::TextDisabled("Overlay / Canvas の常時構成エフェクトは Canvas Editor から編集してください。");
        if (!temporaries_.empty()) {
            ImGui::TextDisabled("Temporary effects: %zu", temporaries_.size());
        }

        ImGui::End();

        // PostEffect ウィンドウを閉じたら PresetEditor / Canvas Editor も閉じる。
        if (!visible) {
            if (presetEditor_) presetEditor_->CloseEditor();
            if (canvasLayerEditor_) canvasLayerEditor_->CloseEditor();
        }
    });

    if (presetEditor_ && presetEditor_->IsOpen()) {
        presetEditor_->ShowEditor();
    }

    // Canvasごとの詳細ウィンドウは常時登録するため、開閉状態に関わらず毎フレーム呼ぶ
    if (canvasLayerEditor_) {
        canvasLayerEditor_->ShowEditor();
    }
}

void PostProcessExecutor::ResizeRenderTextures() {
    chain_->ResizeRenderTextures();
    layer_.ResizeRenderTextures();
    noneCanvas_->ResizeRenderTextures();

#ifdef _DEBUG
    sceneCaptureChain_->ResizeRenderTextures();

    // DebugUI へ再登録する。
    if (debugUI_) {
        sceneImGuiTextureId_ = debugUI_->RegisterTexture(sceneCaptureChain_->GetRenderTextureResource(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    }
#endif
}

IPostEffect* PostProcessExecutor::Create(const std::string& _type) {
    return chain_->Create(_type);
}

uint32_t PostProcessExecutor::CreateTemporary(const std::string& _type) {
    if (!factory_) {
        Log::Send(Log::Level::ERR, "PostEffectFactory is not set");
        return 0;
    }

    auto newEffect = factory_->Create(_type);
    if (!newEffect) {
        Log::Send(Log::Level::ERR, std::format("Failed to create effect type: {}", _type));
        return 0;
    }

    newEffect->SetUp(adapter_, srv_);
    const uint32_t slot = chain_->AllocateRtvSlot();
    newEffect->SetRTVHandle(chain_->GetRtvCpuHandle(slot));
    newEffect->Initialize();

    const uint32_t id = nextTemporaryId_++;
    temporaries_.emplace_back(TemporaryEffectData{ std::move(newEffect), _type, slot, id, true });

    return id;
}

IPostEffect* PostProcessExecutor::GetTemporary(uint32_t _id) {
    for (auto& temporary : temporaries_) {
        if (temporary.id == _id) {
            return temporary.effect.get();
        }
    }
    return nullptr;
}

void PostProcessExecutor::RemoveTemporary(uint32_t _id) {
    for (auto it = temporaries_.begin(); it != temporaries_.end(); ++it) {
        if (it->id == _id) {
            chain_->FreeRtvSlot(it->rtvSlot);
            temporaries_.erase(it);
            return;
        }
    }
}

void PostProcessExecutor::ApplyPreset(const std::string& _presetName, std::function<void()> _onComplete) {
    // 完了時コールバックを保持する。
    onAnimationComplete_ = _onComplete;

    std::string path = PresetsFilePath();

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

    // 今回のアニメーション対象（一時エフェクトのid）を初期化する。
    animatingTemporaries_.clear();

    if (presetData.contains("members")) {
        for (const auto& memberEntry : presetData["members"]) {
            std::string type = memberEntry["type"];

            // preset 名省略時は現在の preset 名を使う。
            std::string presetName = memberEntry.value("preset", _presetName);

            // 演出用の一時エフェクトを生成する。
            const uint32_t id = CreateTemporary(type);
            if (id == 0) {
                Log::Send(Log::Level::WARNING, std::format("Failed to create temporary effect: {}", type));
                continue;
            }

            IPostEffect* effect = GetTemporary(id);
            effect->LoadPreset(presetName);
            Log::Send(Log::Level::DBG, std::format("Loaded preset '{}' for temporary effect ({})", presetName, type));

            animatingTemporaries_.push_back(id);
        }
    }

    // アニメーションありの場合は更新ループへ移行する。
    if (animationDuration_ > 0.0f) {
        isAnimating_ = true;
        animationTimer_ = 0.0f;
        Log::Send(Log::Level::INFO, std::format("Started animation for preset '{}' (duration: {:.1f}s, effects: {})",
            _presetName, animationDuration_, animatingTemporaries_.size()));
    } else {
        // duration が 0 の場合は即座に最終状態を適用してから破棄する。
        for (const uint32_t id : animatingTemporaries_) {
            if (IPostEffect* effect = GetTemporary(id)) {
                effect->UpdateAnimation(1.0f);
            }
            RemoveTemporary(id);
        }
        animatingTemporaries_.clear();
        isAnimating_ = false;

        if (onAnimationComplete_) {
            onAnimationComplete_();
            onAnimationComplete_ = nullptr;
        }
    }

    Log::Send(Log::Level::INFO, std::format("Preset '{}' applied", _presetName));
}

void PostProcessExecutor::Update(float _deltaTime) {
    if (!isAnimating_) return;

    animationTimer_ += _deltaTime;
    float t = std::min(animationTimer_ / animationDuration_, 1.0f);

    // アニメーション対象の一時エフェクトを進行度 t で更新する。
    for (const uint32_t id : animatingTemporaries_) {
        if (IPostEffect* effect = GetTemporary(id)) {
            effect->UpdateAnimation(t);
        }
    }

    // 最後まで到達したら一時エフェクトを破棄する。
    if (t >= 1.0f) {
        isAnimating_ = false;

        for (const uint32_t id : animatingTemporaries_) {
            RemoveTemporary(id);
        }
        animatingTemporaries_.clear();

        Log::Send(Log::Level::INFO, "PostEffect animation completed and temporary effects removed");

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
    for (const auto& effectData : chain_->GetEffects()) {
        if (effectData.enabled) {
            json memberEntry;
            memberEntry["type"] = effectData.type;
            memberEntry["preset"] = _presetName;

            presetJson["members"].push_back(memberEntry);

            // 各 effect のパラメータも保存する。
            effectData.effect->SavePreset(_presetName);
        }
    }

    // duration を保存する。
    presetJson["duration"] = animationDuration_;

    // presets.json を更新する。
    std::string presetsPath = PresetsFilePath();
    std::filesystem::create_directories("./Assets/Data/PostEffect/" + name_);

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

void PostProcessExecutor::LoadPermanentConfig() {
    chain_->LoadPermanentConfig();
}

void PostProcessExecutor::SavePermanentConfig() const {
    chain_->SavePermanentConfig();
}

void PostProcessExecutor::OpenCanvasEditor() {
    if (canvasLayerEditor_) {
        canvasLayerEditor_->OpenEditor();
    }
}

void PostProcessExecutor::OpenPresetEditor(const std::string& _presetName) {
    if (presetEditor_) {
        presetEditor_->OpenEditor(_presetName);
    }
}
