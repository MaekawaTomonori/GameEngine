#include "Engine.hpp"

void Engine::Initialize() {
	windows_ = std::make_unique<Windows>();
}

void Engine::Update() {
	while (windows_->IsEnabled()){
		// Update logic here
		
	}
}

void Engine::Shutdown() {
}

bool Engine::IsEnabled() {
    return windows_->IsEnabled();
}
