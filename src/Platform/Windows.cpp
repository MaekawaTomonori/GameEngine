#include "Windows.hpp"

#include <stdexcept>

void Windows::Initialize() {
	window_ = std::make_unique<Window>();
	if (!window_->Create()) {
		throw std::runtime_error("Failed to create window");
	}
}

bool Windows::IsEnabled() const{
	return window_ ? window_->IsEnabled() : false;
}

void Windows::DisplayLastErr() {
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();

	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&lpMsgBuf),
		0, nullptr) == 0){
		MessageBox(nullptr, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
	}
	MessageBox(nullptr, static_cast<LPCTSTR>(lpMsgBuf), TEXT("Error"), MB_OK);
}
