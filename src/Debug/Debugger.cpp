#ifdef _DEBUG

#include "Debugger.hpp"

#include "DebugUI.hpp"
#include "FrameDebugger.hpp"
#include "PerformanceProfiler.hpp"
#include "WatchDebugger.hpp"
#include "Pattern/Singleton.hpp"

Debugger::Debugger() {
    ui_    = std::make_unique<DebugUI>();
    frame_ = std::make_unique<FrameDebugger>();
}

Debugger::~Debugger() {
    Singleton<WatchDebugger>::GetInstance()->Finalize();
}

void Debugger::Initialize(GESTD::ReferencePtr<DirectXAdapter> _adapter) {
    ui_->Initialize(_adapter);
    frame_->Initialize(GESTD::ReferencePtr<DebugUI>(ui_));
    Singleton<PerformanceProfiler>::GetInstance()->Initialize(ui_.get());
    Singleton<WatchDebugger>::GetInstance()->Initialize(GESTD::ReferencePtr<DebugUI>(ui_));
}

void Debugger::SetStopCallback(std::function<void()> _cb) {
    frame_->SetStopCallback(std::move(_cb));
}

bool Debugger::ShouldUpdate() {
    return frame_->ShouldUpdate();
}

void Debugger::Debug() {
    Singleton<PerformanceProfiler>::GetInstance()->Debug();
    Singleton<WatchDebugger>::GetInstance()->Debug();
}

void Debugger::Render() {
    ui_->Render();
}

void Debugger::UpdateDisplaySize(int _width, int _height) {
    ui_->UpdateDisplaySize(_width, _height);
}

#endif // _DEBUG
