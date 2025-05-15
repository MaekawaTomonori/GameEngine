#pragma once
#include <memory>

#include "Platform/Windows.hpp"

class Engine {
private:
	std::unique_ptr<Windows> windows_;
public:
	void Initialize();
	void Update();
	void Shutdown();

	bool IsEnabled();
};
