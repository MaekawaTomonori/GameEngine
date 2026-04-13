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

    if (FAILED(keyboard_->SetCooperativeLevel(hWnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
        Log::Send(Log::Level::ERR, "Failed to set keyboard cooperative level.");
        Utils::Alert("Failed to set keyboard cooperative level.");
        return;
    }

    // Mouse Setup
    if (FAILED(directInput_->CreateDevice(GUID_SysMouse, &mouse_, NULL))) {
        Log::Send(Log::Level::ERR, "Failed to create mouse device.");
        Utils::Alert("Failed to create mouse device.");
        return;
    }

    if (FAILED(mouse_->SetDataFormat(&c_dfDIMouse))) {
        Log::Send(Log::Level::ERR, "Failed to set mouse data format.");
        Utils::Alert("Failed to set mouse data format.");
        return;
    }

    if (FAILED(mouse_->SetCooperativeLevel(hWnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
        Log::Send(Log::Level::ERR, "Failed to set mouse cooperative level.");
        Utils::Alert("Failed to set mouse cooperative level.");
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
    UpdateMouse();
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

void Input::UpdateMouse() {
    if (mouse_) {
        mouse_->Acquire();
        preMouseState_ = mouseState_;
        mouse_->GetDeviceState(sizeof(mouseState_), &mouseState_);

        // スクリーン座標を取得 (Windows API)
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(hWnd_, &point);
        mousePosition_ = { static_cast<float>(point.x), static_cast<float>(point.y) };
    }
}

Vector2 Input::GetMousePosition() const {
#ifdef _DEBUG
    if (sceneViewTransform_.active
        && sceneViewTransform_.imageSize.x > 0.f
        && sceneViewTransform_.imageSize.y > 0.f) {
        // ImGui画像内の相対位置をゲーム解像度にマッピング
        const Vector2 relative = {
            mousePosition_.x - sceneViewTransform_.imagePos.x,
            mousePosition_.y - sceneViewTransform_.imagePos.y
        };
        return {
            relative.x * sceneViewTransform_.gameSize.x / sceneViewTransform_.imageSize.x,
            relative.y * sceneViewTransform_.gameSize.y / sceneViewTransform_.imageSize.y
        };
    }
#endif
    return mousePosition_;
}

void Input::SetSceneViewTransform(bool _active, Vector2 _imagePos, Vector2 _imageSize, Vector2 _gameSize) {
    sceneViewTransform_ = { _active, _imagePos, _imageSize, _gameSize };
}

void Input::SetCursorVisible(const bool _visible) {
    cursorVisible_ = _visible;
}

void Input::SetDebugUIHovered(const bool _hovered) {
    debugUIHovered_ = _hovered;
}

void Input::ApplyCursorVisibility() {
    bool shouldShow = cursorVisible_;

    // タイトルバーなどのノンクライアント領域ではカーソルを常に表示する
    if (!shouldShow) {
        POINT ptScreen;
        GetCursorPos(&ptScreen);
        RECT clientRect;
        GetClientRect(hWnd_, &clientRect);
        POINT topLeft = { 0, 0 };
        ClientToScreen(hWnd_, &topLeft);
        const bool inClientArea =
            ptScreen.x >= topLeft.x &&
            ptScreen.x <  topLeft.x + clientRect.right &&
            ptScreen.y >= topLeft.y &&
            ptScreen.y <  topLeft.y + clientRect.bottom;
        if (!inClientArea) {
            shouldShow = true;
        }
    }

#ifdef _DEBUG
    if (!cursorVisible_) {
        if (sceneViewTransform_.active
            && sceneViewTransform_.imageSize.x > 0.f
            && sceneViewTransform_.imageSize.y > 0.f) {
            // スクリーン座標で SceneView ヒットテスト（imagePos は GetItemRectMin() 由来）
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd_, &pt);
            const float px = static_cast<float>(pt.x);
            const float py = static_cast<float>(pt.y);
            const bool insideScene =
                px >= sceneViewTransform_.imagePos.x &&
                px <  sceneViewTransform_.imagePos.x + sceneViewTransform_.imageSize.x &&
                py >= sceneViewTransform_.imagePos.y &&
                py <  sceneViewTransform_.imagePos.y + sceneViewTransform_.imageSize.y;
            // SceneView 画像内でも DebugUI パネルが手前にある場合はカーソルを表示する
            shouldShow = !insideScene || debugUIHovered_;
        }
    }
#endif

    if (shouldShow != appliedCursorVisible_) {
        ShowCursor(shouldShow ? TRUE : FALSE);
        appliedCursorVisible_ = shouldShow;
    }
}
