#include "Window.hpp"

#include <stdexcept>

LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg){
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			break;
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
	RegisterClass(&wc);

	//LastErr();

	RECT rect = {0, 0, 800, 600};

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window
	hWnd_ = CreateWindow(
		wc.lpszClassName,
		title_.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	if (!hWnd_){
		LastErr();
		return false;
	}

	ShowWindow(hWnd_, SW_SHOW);

	return true;
	//LastErr();
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

