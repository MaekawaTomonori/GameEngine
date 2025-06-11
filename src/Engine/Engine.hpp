#ifndef Engine_HPP_
#define Engine_HPP_

#include <memory>

#include "Config/Config.hpp"
#include "include/Input.hpp"

#include "src/Platform/WinApp.hpp"
#include "src/Renderer/DirectX/DirectXAdapter.hpp"
#include "DebugUI.hpp"
#include "src/Renderer/Renderer.hpp"
#include "src/Timer/Timer.hpp"

class Engine {
    GameEngine::Config* config_ = nullptr;
    std::unique_ptr<WinApp> windows_;
    std::unique_ptr<DirectXAdapter> dxAdaptor_;
    std::unique_ptr<Renderer> renderer_;
    //std::unique_ptr<DebugUI> debugUI_;

    Input* input_ = nullptr;

    std::unique_ptr<Timer> timer_;

    const uint16_t UpdateRate = 60; // Default update rate
public:
    Engine();
    ~Engine() = default;
    void Initialize();
    void Update();
    void Render();
    void Shutdown();

    bool IsEnabled() const;

    bool UpdateSkip() const;

    void ApplyConfig(GameEngine::Config* _config);
};

#endif

