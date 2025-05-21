#include "WinApp.hpp"

#include <stdexcept>

void WinApp::Initialize() {
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	window_ = std::make_unique<Window>();
	if (!window_->Create()) {
		throw std::runtime_error("Failed to create window");
	}
}

bool WinApp::IsEnabled() const{
	return window_ ? window_->IsEnabled() : false;
}


