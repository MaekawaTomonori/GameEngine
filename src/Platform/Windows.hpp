#ifndef Windows_HPP_
#define Windows_HPP_
#include <memory>

#include "Window/Window.hpp"

class Windows {
	bool enabled_ = true;
	std::unique_ptr<Window> window_;
public:
	void Initialize();

	bool IsEnabled() const ;
private:
}; // class Windows

#endif // Windows_HPP_
