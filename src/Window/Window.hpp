#ifndef Window_HPP_
#define Window_HPP_

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <Windows.h>
#include <string>


class Window {
public:
	void Create();
	bool IsEnabled();
private:
	std::string title_;

	HWND hWnd_ = nullptr;
	HINSTANCE hInstance_ = nullptr;


}; // class Window

#endif // Window_HPP_
