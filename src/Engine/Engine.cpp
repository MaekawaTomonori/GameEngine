#include "Engine.hpp"

Engine::Engine() {

}

void Engine::Initialize() {
	windows_ = std::make_unique<Windows>();
	windows_->Initialize();
}

void Engine::Update() {

}

void Engine::Shutdown() {
}

bool Engine::IsEnabled() const {
    return windows_ ? windows_->IsEnabled() : false;
}
