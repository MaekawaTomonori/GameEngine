#ifndef WinApp_HPP_
#define WinApp_HPP_
#include <memory>

#include "src/Window/Window.hpp"

class WinApp {
	std::unique_ptr<Window> window_;
public:
	~WinApp() = default;
	void Initialize();

	bool IsEnabled() const ;

	HWND GetWindowHandle() const;

private:
	HWND hWnd_ = nullptr;
}; // class WinApp

#endif // WinApp_HPP_
