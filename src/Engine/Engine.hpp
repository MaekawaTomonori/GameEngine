#ifndef Engine_HPP_
#define Engine_HPP_

#include <memory>

#include "Platform/Windows.hpp"

class Engine {
	std::unique_ptr<Windows> windows_;
public:
	Engine() = default;
	~Engine() = default;
	void Initialize();
	void Update();
	void Shutdown();

	bool IsEnabled();
};

#endif

