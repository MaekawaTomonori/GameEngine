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

private:
}; // class WinApp

#endif // WinApp_HPP_
