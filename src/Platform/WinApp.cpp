#include "WinApp.hpp"

#include <stdexcept>

#include "include/Utils.hpp"

void WinApp::Initialize() {
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	window_ = std::make_unique<Window>();
	if (!window_->Create()) {
		throw std::runtime_error("Failed to create window");
	}
	hWnd_ = window_->GetWindowHandle();
	if (!hWnd_){
		Utils::DisplayLastErr();
		throw std::runtime_error("Failed to get window handle");
	}
}

bool WinApp::IsEnabled() const{
	return window_ ? window_->IsEnabled() : false;
}

HWND WinApp::GetWindowHandle() const {
	return hWnd_;
}


