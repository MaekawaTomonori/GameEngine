#include "Renderer.hpp"

#include "Log.hpp"
#include "src/Canvas/Canvas.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/PostProcess/Executor/PostProcessExecutor.hpp"

void Renderer::Initialize(GESTD::ReferencePtr<DirectXAdapter> _adapter, GESTD::ReferencePtr<PostProcessExecutor> _postProcessor) {
    adapter_ = _adapter;
    postProcessor_ = _postProcessor;
}

void Renderer::Register(std::function<void()> _task, const std::string& _canvasName) {
    Canvas* canvas = postProcessor_->GetCanvas(_canvasName);
    if (!canvas) {
        Log::Send(Log::Level::ERR, "Renderer: Canvas not found: " + _canvasName + ", falling back to None");
        canvas = postProcessor_->GetCanvas("None");
    }

    if (canvas) {
        canvas->RegisterTask(std::move(_task));
    }
}

void Renderer::RegisterUI(std::function<void()> _task) {
    uiTasks_.push(std::move(_task));
}

void Renderer::Render() {
    // Phase 1: 各Canvasが自分のタスクを自分のRTへ描画し、常時構成PostEffectを適用する
    postProcessor_->DrawObjects();
    postProcessor_->ApplyPostEffects();

    // Phase 2: 各Canvasを合成し、ワンショット演出を適用する（自分のRTへ描画するだけ）
    postProcessor_->DrawCanvases();

    // Phase 3: スワップチェーンへ切り替えてから、最終結果を描画する
    adapter_->BeginFrame();
    postProcessor_->Draw();

    // UIタスク（ImGui等）は常にスワップチェーンへ描画
    while (!uiTasks_.empty()) {
        auto task = std::move(uiTasks_.front());
        uiTasks_.pop();
        task();
    }

    adapter_->EndFrame();
}
