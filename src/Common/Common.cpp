#include "Common.hpp"

#include "src/Renderer/Renderer.hpp"

void Common::Setup(DirectXAdapter* _adapter, DebugUI* _debugUi) {
    std::lock_guard<std::mutex> lock(mutex_);
    adapter_ = _adapter;
    debugUI_ = _debugUi;
}

void Common::Draw(Renderer* _renderer) {
    std::vector<std::function<void()>> postEffectTasks;
    std::vector<std::function<void()>> noPostEffectTasks;
    {
        for (const auto& command : drawFunctions_){
            if (!command.func)continue;
            if (command.applyPostEffects){
                postEffectTasks.push_back(command.func);
            } else {
                noPostEffectTasks.push_back(command.func);
            }
        }
        drawFunctions_.clear();
    }

    if (!pipeline_) return;
    if (!noPostEffectTasks.empty()) {
        _renderer->Register([this, noPostEffectTasks](){
            pipeline_->DrawCall();
            for (auto& task : noPostEffectTasks){
                task();
            }
        });
    }

    if (!postEffectTasks.empty()) {
        _renderer->Register([this, postEffectTasks](){
            pipeline_->DrawCall();

            for (auto& task : postEffectTasks){
                task();
            }
        }, true);
    }
}

void Common::RegisterDebug(const std::string &_id, const std::function<void()> &_command) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (debugUI_){
        debugUI_->RegisterCommand(_id, _command);
    }
}

void Common::RegisterDraw(const std::function<void()>& _command, bool _isApplyPostEffect) {
    drawFunctions_.push_back({ _command, _isApplyPostEffect });
}
