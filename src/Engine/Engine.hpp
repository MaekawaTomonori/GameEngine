#ifndef Engine_HPP_
#define Engine_HPP_

#include <memory>

#include "include/Input.hpp"
#include "src/DirectX/DirectXAdaptor.hpp"
#include "src/Platform/WinApp.hpp"

class Engine {
	std::unique_ptr<WinApp> windows_;
	std::unique_ptr<DirectXAdaptor> dxAdaptor_;

	Input* input_ = nullptr;
public:
	Engine();
	~Engine() = default;
	void Initialize();
	void Update();
	void Shutdown();

	bool IsEnabled() const;
};

#endif

