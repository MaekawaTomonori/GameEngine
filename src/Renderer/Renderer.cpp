#include "Renderer.hpp"

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/PostProcess/Executor/PostProcessExecutor.hpp"

void Renderer::Initialize(DirectXAdapter* _adapter, PostProcessExecutor* _postProcessor) {
    adapter_ = _adapter;
    postProcessor_ = _postProcessor;
}

void Renderer::Register(const std::function<void()>& _task, const bool _applyPostEffect) {
    if (_applyPostEffect){
        pp_.push(std::move(_task));
    } else{
        tasks_.push(std::move(_task));
    }
}

void Renderer::RegisterUI(const std::function<void()>& _task) {
    uiTasks_.push(std::move(_task));
}

void Renderer::Render() {
    // キャプチャモード: Scene ウィンドウが表示されている場合、全オブジェクトを renderTexture_ へ描画
    const bool captureMode = postProcessor_ && postProcessor_->IsSceneViewActive();

    // Phase 1: シーン描画（renderTexture_）
    // キャプチャモード時は非PostEffectタスクも renderTexture_ へ描画する
    bool hasPostProcess = false;
    if (!pp_.empty() || (captureMode && !tasks_.empty())) {
        postProcessor_->BeginFrame();
        hasPostProcess = true;

        while (!pp_.empty()) {
            auto task = std::move(pp_.front());
            pp_.pop();
            task();
        }

        if (captureMode) {
            // 非PostEffectゲームオブジェクト（Sprite等）もrenderTexture_へ描画
            while (!tasks_.empty()) {
                auto task = std::move(tasks_.front());
                tasks_.pop();
                task();
            }
        }

        postProcessor_->EndFrame();
        postProcessor_->Execute();
    }

    // Phase 2: スワップチェーンへ描画（常に実行してフレームをPresent）
    adapter_->BeginFrame();

    if (hasPostProcess && !captureMode) {
        // 通常モード: PostProcess結果をスワップチェーンへ描画
        postProcessor_->Draw();
    }

    if (!captureMode) {
        // 通常モード: 非PostEffectゲームオブジェクトをスワップチェーンへ描画
        while (!tasks_.empty()) {
            auto task = std::move(tasks_.front());
            tasks_.pop();
            task();
        }
    }

    // UIタスク（ImGui等）は常にスワップチェーンへ描画
    while (!uiTasks_.empty()) {
        auto task = std::move(uiTasks_.front());
        uiTasks_.pop();
        task();
    }

    adapter_->EndFrame();
}
