#include "Engine.hpp"
#include "include/Singleton.hpp"
#include <stdexcept>

Engine::Engine() {
}

void Engine::Initialize() {
	config_ = GameEngine::Config::Default();

	windows_ = std::make_unique<WinApp>();
	windows_->Initialize();

	dxAdaptor_ = std::make_unique<DirectXAdapter>(windows_->GetWindowHandle(), config_->GetWidth(), config_->GetHeight());

	input_ = Singleton<Input>::GetInstance();
	input_->Initialize();
}

void Engine::Update() {
	if (input_){
		input_->Update();
	}
}

void Engine::Render() {
	if (!dxAdaptor_){
		throw std::runtime_error("DirectXAdapter is not initialized");
	}
	dxAdaptor_->Render();
}

void Engine::Shutdown() {
}

bool Engine::IsEnabled() const {
	return windows_ ? windows_->IsEnabled() : false;
}

void Engine::ApplyConfig(GameEngine::Config *_config) {
	config_ = _config;
}
