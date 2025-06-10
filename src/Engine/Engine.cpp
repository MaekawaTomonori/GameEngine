#include "Engine.hpp"
#include "include/Singleton.hpp"
#include <stdexcept>

#include "Utils.hpp"

Engine::Engine() = default;

void Engine::Initialize() {
    config_ = GameEngine::Config::Default();

    windows_ = std::make_unique<WinApp>();
    windows_->Initialize();
    //windows_->SetWindowSize(static_cast<int>(config_->GetWidth()), static_cast<int>(config_->GetHeight()));

    dxAdaptor_ = std::make_unique<DirectXAdapter>(windows_->GetWindowHandle(), config_->GetWidth(), config_->GetHeight());

    renderer_ = std::make_unique<Renderer>();
    renderer_->Initialize(dxAdaptor_.get());

    debugUI_ = std::make_unique<DebugUI>();
    debugUI_->Initialize(dxAdaptor_.get());

    input_ = Singleton<Input>::GetInstance();
    input_->Initialize();

    timer_ = std::make_unique<Timer>(static_cast<std::chrono::milliseconds>(static_cast<uint64_t>(1e4 / UpdateRate)));
    timer_->Start();
}

void Engine::Update() {
    if (input_){
        input_->Update();
    }
    if (debugUI_) {
	    debugUI_->Process();
    }
}

void Engine::Render() {
    if (!dxAdaptor_){
	    Utils::Alert("DirectXAdapter is not initialized");
    }
    renderer_->Register([&](){debugUI_->Render(); });
    renderer_->Render();
}

void Engine::Shutdown() {
    
}

bool Engine::IsEnabled() const {
    return windows_ ? windows_->IsEnabled() : false;
}

bool Engine::UpdateSkip() const {
    if (!timer_) return false;
    const bool s = timer_->Check();
    if (s){
        timer_->Restart();
    }
    return !s;
}

void Engine::ApplyConfig(GameEngine::Config* _config) {
    config_ = _config;
}
