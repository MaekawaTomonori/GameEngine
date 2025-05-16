#include "Windows.hpp"

#include <stdexcept>

void Windows::Initialize() {
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	window_ = std::make_unique<Window>();
	if (!window_->Create()) {
		throw std::runtime_error("Failed to create window");
	}
}

bool Windows::IsEnabled() const{
	return window_ ? window_->IsEnabled() : false;
}


