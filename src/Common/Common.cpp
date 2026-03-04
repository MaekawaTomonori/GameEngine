#include "Common.hpp"

#include <ranges>

#ifdef _DEBUG
#include "imgui.h"
#endif

#include "src/Renderer/Renderer.hpp"

void Common::Setup(DirectXAdapter* _adapter, DebugUI* _debugUi, const std::string& _windowName) {
    std::scoped_lock lock(mutex_);
    adapter_ = _adapter;
    debugUI_ = _debugUi;
    windowName_ = _windowName;
}

void Common::Update() {
    if (updateCommands_.empty()) return;

    for (const auto& func : updateCommands_ | std::views::values) {
        func();
    }
}

void Common::Debug() {
    if (!debugUI_) return;

    debugUI_->RegisterCommand(windowName_, [this]() {
#ifdef _DEBUG
        ImGui::Begin(windowName_.c_str());
        for (const auto& func : debugCommands_ | std::views::values) {
            func();
        }
        ImGui::End();
#endif
    });
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

void Common::RegisterDebug(const std::string& _id, const std::function<void()>& _func) {
    std::lock_guard lock(mutex_);

    debugCommands_[_id] = _func;
}

void Common::RegisterUpdate(const std::string& _id, const std::function<void()>& _func) {
    std::lock_guard lock(mutex_);

    updateCommands_[_id] = _func;
}

void Common::RegisterDraw(const std::function<void()>& _command, bool _isApplyPostEffect) {
    drawFunctions_.push_back({ _command, _isApplyPostEffect });
}

void Common::Unregister(const std::string& _uuid) {
    std::lock_guard lock(mutex_);
    updateCommands_.erase(_uuid);
    debugCommands_.erase(_uuid);
}
