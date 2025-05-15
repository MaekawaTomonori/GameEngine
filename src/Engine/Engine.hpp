#ifndef Engine_HPP_
#define Engine_HPP_

#include <memory>

#include "src/Platform/Windows.hpp"

class Engine {
	std::unique_ptr<Windows> windows_;
public:
	Engine();
	~Engine() = default;
	void Initialize();
	void Update();
	void Shutdown();

	bool IsEnabled() const;
};

#endif

