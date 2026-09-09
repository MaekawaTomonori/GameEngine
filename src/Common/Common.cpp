#include "Common.hpp"

#include <algorithm>

#ifdef _DEBUG
#include "imgui.h"
#endif

#include "src/Renderer/Renderer.hpp"

void Common::Setup(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi, const std::string& _windowName) {
    std::scoped_lock lock(mutex_);
    adapter_ = _adapter;
    debugUI_ = _debugUi;
    windowName_ = _windowName;

    debugUI_->RegisterMenuButton(windowName_, false, "ObjectCommons");
}

void Common::Update() {
    for (const auto& command : updateCommands_) {
        command.func();
    }
}

void Common::Debug() {
    if (!debugUI_) return;

    debugUI_->RegisterCommand(windowName_, [this]() {
#ifdef _DEBUG
        ImGui::Begin(windowName_.c_str(), &debugUI_->IsVisible(windowName_));
        for (const auto& command : debugCommands_) {
            command.func();
        }
        ImGui::End();
#endif
    });
}

void Common::Draw(Renderer* _renderer) {
    std::vector<std::function<void()>> tasks;
    {
        for (auto& command : drawFunctions_){
            if (!command.func)continue;
            tasks.push_back(std::move(command.func));
        }
        drawFunctions_.clear();
    }

    if (!pipeline_) return;
    if (!tasks.empty()) {
        _renderer->Register([this, tasks = std::move(tasks)](){
            pipeline_->DrawCall();
            for (auto& task : tasks){
                task();
            }
        }, "None");
    }
}

void Common::RegisterDebug(const std::string& _id, const std::function<void()>& _func) {
    std::lock_guard lock(mutex_);

    debugCommands_.push_back({ _id, _func });
}

void Common::RegisterUpdate(const std::string& _id, const std::function<void()>& _func) {
    std::lock_guard lock(mutex_);

    updateCommands_.push_back({ _id, _func });
}

void Common::RegisterDraw(const std::function<void()>& _command) {
    drawFunctions_.push_back({ _command });
}

void Common::Unregister(const std::string& _uuid) {
    std::lock_guard lock(mutex_);
    std::erase_if(updateCommands_, [&_uuid](const KeyedCommand& _command) { return _command.id == _uuid; });
    std::erase_if(debugCommands_, [&_uuid](const KeyedCommand& _command) { return _command.id == _uuid; });
}
