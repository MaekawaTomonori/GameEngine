#ifndef Window_HPP_
#define Window_HPP_

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <Windows.h>
#include <string>

class Window {
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Create();
	bool IsEnabled();
	void LastErr();
private:
	std::wstring title_ = L"Title";

	HWND hWnd_ = nullptr;
	HINSTANCE hInstance_ = nullptr;


}; // class Window

#endif // Window_HPP_
