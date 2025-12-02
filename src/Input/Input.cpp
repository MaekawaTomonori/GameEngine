#include "include/Input.hpp"

#include "Log.hpp"
#include "Utils.hpp"
#include "imgui.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(HWND _hWnd, HINSTANCE _hInstance) {
    hWnd_ = _hWnd;
    hInstance_ = _hInstance;

    if (FAILED(DirectInput8Create(hInstance_, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr))) {
        Log::Send(Log::Level::ERR, "Failed to create DirectInput8 instance.");
        Utils::Alert("Failed to create DirectInput8 instance.");
        return;
    }

    if (FAILED(directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL))) {
        Log::Send(Log::Level::ERR, "Failed to create keyboard device.");
        Utils::Alert("Failed to create keyboard device.");
        return;
    }

    if (FAILED(keyboard_->SetDataFormat(&c_dfDIKeyboard))) {
        Log::Send(Log::Level::ERR, "Failed to set keyboard data format.");
        Utils::Alert("Failed to set keyboard data format.");
        return;
    }

    if (FAILED(keyboard_->SetCooperativeLevel(hWnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY))) {
        Log::Send(Log::Level::ERR, "Failed to set keyboard cooperative level.");
        Utils::Alert("Failed to set keyboard cooperative level.");
        return;
    }
}

void Input::Update() {
#ifdef _DEBUG
    const ImGuiIO& io = ImGui::GetIO();

    // Keyboard update
    if (!io.WantCaptureKeyboard) 
#endif
        UpdateKeyboard();

    // Mouse 
}

bool Input::IsPress(const BYTE _key) const {
    return keyState_[_key];
}

bool Input::IsTrigger(const BYTE _key) const {
    return keyState_[_key] && !preState_[_key];
}

void Input::UpdateKeyboard() {
    if (keyboard_) {
        keyboard_->Acquire();
        memcpy_s(preState_, sizeof(keyState_), keyState_, sizeof(keyState_));
        keyboard_->GetDeviceState(sizeof(keyState_), keyState_);
    }
}
