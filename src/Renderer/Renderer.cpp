#include "Renderer.hpp"

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/PostProcess/Executor/PostProcessExecutor.hpp"

void Renderer::Initialize(DirectXAdapter* _adapter, PostProcessExecutor* _postProcessor) {
    adapter_ = _adapter;
    postProcessor_ = _postProcessor;
}

void Renderer::Register(std::function<void()> _task, RenderPhase _phase) {
    switch (_phase) {
    case RenderPhase::Scene:
        sceneTasks_.push(_task);
        break;
    case RenderPhase::UI:
        uiTasks_.push(_task);
        break;
    case RenderPhase::PostEffect:
        // PostEffectは直接実行されるため、タスクキューには登録しない
        break;
    }
}

void Renderer::Render() {
    adapter_->BeginFrame();
    
    // 1. シーン描画フェーズ
    RenderScene();
    
    // 2. PostEffect適用フェーズ
    ApplyPostEffects();
    
    // 3. UI描画フェーズ
    RenderUI();
    
    // フレーム終了処理（Present含む）
    adapter_->EndFrame();
}

void Renderer::RenderScene() {
    if (sceneTasks_.empty()) return;
    
    // PostProcessor用RenderTextureに描画開始
    if (postProcessor_) {
        SetSceneRenderTarget();
    } else {
        // PostProcessorがない場合は直接スワップチェーンに
        SetSwapChainRenderTarget();
    }
    
    // シーンタスク実行
    while (!sceneTasks_.empty()) {
        auto task = sceneTasks_.front();
        sceneTasks_.pop();
        if (task) {
            task();
        }
    }
    
    // PostProcessor用キャプチャ終了
    if (postProcessor_) {
        postProcessor_->EndSceneCapture();
    }
}

void Renderer::ApplyPostEffects() {
    if (!postProcessor_) return;
    
    // PostEffectチェーンを実行
    postProcessor_->Execute();
    
    // スワップチェーンをRenderTargetに設定
    SetSwapChainRenderTarget();
    
    // 最終的なOffscreenQuadを描画
    postProcessor_->Draw();
}

void Renderer::RenderUI() {
    if (uiTasks_.empty()) return;
    
    // 既にスワップチェーンがRenderTargetに設定済み
    while (!uiTasks_.empty()) {
        auto task = uiTasks_.front();
        uiTasks_.pop();
        if (task) {
            task();
        }
    }
}

void Renderer::SetSceneRenderTarget() {
    if (postProcessor_) {
        postProcessor_->BeginSceneCapture();
    }
}

void Renderer::SetSwapChainRenderTarget() {
    adapter_->SetSwapChainRenderTarget();
}