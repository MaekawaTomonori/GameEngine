#include "Windows.hpp"

void Windows::Initialize() {
	window_ = std::make_unique<Window>();
	window_->Create();
}

bool Windows::IsEnabled() const{
	return window_->IsEnabled();
}
