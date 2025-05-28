#ifndef Engine_HPP_
#define Engine_HPP_

#include <memory>

#include "Config/Config.hpp"
#include "include/Input.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/Platform/WinApp.hpp"

class Engine {
	GameEngine::Config* config_ = nullptr;
	std::unique_ptr<WinApp> windows_;
	std::unique_ptr<DirectXAdapter> dxAdaptor_;

	Input* input_ = nullptr;
public:
	Engine();
	~Engine() = default;
	void Initialize();
	void Update();
	void Render();
	void Shutdown();

	bool IsEnabled() const;

	void ApplyConfig(GameEngine::Config* _config);
};

#endif

