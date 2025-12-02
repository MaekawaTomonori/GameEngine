#include "Window.hpp"

#include <stdexcept>

#include "include/Utils.hpp"
#include "vendor/imgui/imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#ifdef _DEBUG
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))return true;
#endif

    switch (uMsg){
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool Window::Create() {
    // Register the window class
    hInstance_ = GetModuleHandle(nullptr);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = title_.c_str();
    wc.hInstance = hInstance_;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClass(&wc)){
        Utils::DisplayLastErr();
        return false;
    }

    RECT rect = {0, 0, 1280, 720};

    DWORD windowStyle = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;

    AdjustWindowRect(&rect, windowStyle, FALSE);

    // Create the window
    hWnd_ = CreateWindow(
        wc.lpszClassName,
        title_.c_str(),
        windowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInstance_,
        nullptr
    );

    if (!hWnd_){
        Utils::DisplayLastErr();
        return false;
    }

    GetClientRect(hWnd_, &previousRect_);
    previousStyle_ = windowStyle;

    ShowWindow(hWnd_, SW_SHOW);
    UpdateWindow(hWnd_);

    return true;
}

[[nodiscard]]
bool Window::IsEnabled() {
    MSG msg{};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)){
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

void Window::BorderlessFullScreen() {
    if (!hWnd_) return;
    
    previousStyle_ = GetWindowLong(hWnd_, GWL_STYLE);
    GetClientRect(hWnd_, &previousRect_);
    
    // ウィンドウスタイル変更（ボーダレス）
    SetWindowLongPtr(hWnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    
    // モニタ情報取得
    HMONITOR hMon = MonitorFromWindow(hWnd_, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    if (!GetMonitorInfo(hMon, &mi)) {
        Utils::DisplayLastErr();
        return;
    }
    
    // スタイル変更を即座に反映してフルスクリーン化
    if (!SetWindowPos(
        hWnd_,
        HWND_TOP,
        mi.rcMonitor.left,
        mi.rcMonitor.top,
        mi.rcMonitor.right - mi.rcMonitor.left,
        mi.rcMonitor.bottom - mi.rcMonitor.top,
        SWP_FRAMECHANGED | SWP_NOOWNERZORDER
    )) {
        Utils::DisplayLastErr();
    }

    ShowWindow(hWnd_, SW_SHOW);
    UpdateWindow(hWnd_);

    // フォーカスを維持
    SetForegroundWindow(hWnd_);
    SetFocus(hWnd_);
}

void Window::ToggleBorderless() {
    if (isBorderless_) {
        RestoreWindowMode();
        isBorderless_ = false;
    } else {
        BorderlessFullScreen();
        isBorderless_ = true;
    }
    // Note: DirectXAdapterとImGuiの更新はFramework::Update()で行われる
}

void Window::RestoreWindowMode() const {
    if (!hWnd_) return;

    // リサイズ不可のウィンドウスタイルに戻す
    SetWindowLongPtr(hWnd_, GWL_STYLE, previousStyle_);

    // 元のサイズに戻す
    RECT rect = previousRect_;
    AdjustWindowRect(&rect, previousStyle_, FALSE);

    // ウィンドウを中央に配置
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    if (!SetWindowPos(
        hWnd_,
        HWND_NOTOPMOST,
        (screenWidth - windowWidth) / 2,
        (screenHeight - windowHeight) / 2,
        windowWidth,
        windowHeight,
        SWP_FRAMECHANGED | SWP_NOOWNERZORDER
    )) {
        Utils::DisplayLastErr();
    }
}

HWND Window::GetWindowHandle() const {
    return hWnd_;
}

HINSTANCE Window::GetInstanceHandle() const {
    return hInstance_;
}

void Window::SetSize(const int width, const int height) const {
    if (hWnd_){
        RECT rect = {0, 0, width, height};

        SetWindowPos(hWnd_, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void Window::SetTitle(const std::string& _title) {
    title_ = Utils::Convert(_title);
}

