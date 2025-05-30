#include "Engine.hpp"
#include "include/Singleton.hpp"
#include <stdexcept>

Engine::Engine() = default;

void Engine::Initialize() {
	config_ = GameEngine::Config::Default();

	windows_ = std::make_unique<WinApp>();
	windows_->Initialize();
	//windows_->SetWindowSize(static_cast<int>(config_->GetWidth()), static_cast<int>(config_->GetHeight()));

	dxAdaptor_ = std::make_unique<DirectXAdapter>(windows_->GetWindowHandle(), config_->GetWidth(), config_->GetHeight());

	debugUI_ = std::make_unique<DebugUI>();
	debugUI_->Initialize(dxAdaptor_.get());

	input_ = Singleton<Input>::GetInstance();
	input_->Initialize();
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
		throw std::runtime_error("DirectXAdapter is not initialized");
	}
	dxAdaptor_->Register([&](){debugUI_->Render(); });
	dxAdaptor_->Render();
}

void Engine::Shutdown() {

}

bool Engine::IsEnabled() const {
	return windows_ ? windows_->IsEnabled() : false;
}

void Engine::ApplyConfig(GameEngine::Config* _config) {
	config_ = _config;
}
