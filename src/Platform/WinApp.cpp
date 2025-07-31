#include "WinApp.hpp"

#include <stdexcept>

#include "include/Utils.hpp"

#pragma comment(lib, "winmm.lib")

void WinApp::Initialize() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    timeBeginPeriod(1);
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

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

void WinApp::SetWindowSize(const int width, const int height) const {
    if (window_){
        window_->SetSize(width, height);
    }
}

void WinApp::SetTitle(const std::string& _title) const {
    window_->SetTitle(_title);
}


