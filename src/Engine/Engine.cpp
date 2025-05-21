#include "Engine.hpp"
#include "include/Singleton.hpp"

Engine::Engine() {
}

void Engine::Initialize() {
	windows_ = std::make_unique<WinApp>();
	windows_->Initialize();

	input_ = Singleton<Input>::GetInstance();
	input_->Initialize();
}

void Engine::Update() {
	if (input_){
		input_->Update();
	}
}

void Engine::Shutdown() {
}

bool Engine::IsEnabled() const {
	return windows_ ? windows_->IsEnabled() : false;
}
