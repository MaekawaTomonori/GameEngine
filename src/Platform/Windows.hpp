#ifndef Windows_HPP_
#define Windows_HPP_
#include <memory>

#include "src/Window/Window.hpp"

class Windows {
	std::unique_ptr<Window> window_;
public:
	~Windows() = default;
	void Initialize();

	bool IsEnabled() const ;

	static void DisplayLastErr();
private:
}; // class Windows

#endif // Windows_HPP_
