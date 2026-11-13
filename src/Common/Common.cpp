#include "Common.hpp"

#include <algorithm>
#include <map>

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
    // Canvas ごとに深度がクリアされるので、深度で前後を判定したいものは描き先を分けて登録する
    std::map<std::string, std::vector<std::function<void()>>> tasksByCanvas;
    {
        for (auto& command : drawFunctions_){
            if (!command.func)continue;
            tasksByCanvas[command.canvasName].push_back(std::move(command.func));
        }
        drawFunctions_.clear();
    }

    if (!pipeline_) return;
    for (auto& [canvasName, tasks] : tasksByCanvas) {
        _renderer->Register([this, tasks = std::move(tasks)](){
            pipeline_->DrawCall();
            for (auto& task : tasks){
                task();
            }
        }, canvasName);
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

void Common::RegisterDraw(const std::function<void()>& _command, const std::string& _canvasName) {
    drawFunctions_.push_back({ _command, _canvasName });
}

void Common::Unregister(const std::string& _uuid) {
    std::lock_guard lock(mutex_);
    std::erase_if(updateCommands_, [&_uuid](const KeyedCommand& _command) { return _command.id == _uuid; });
    std::erase_if(debugCommands_, [&_uuid](const KeyedCommand& _command) { return _command.id == _uuid; });
}
