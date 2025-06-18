#ifndef Framework_HPP_
#define Framework_HPP_

#include <memory>

#include "Input.hpp"
#include "src/Platform/WinApp.hpp"
#include "src/Config/Config.hpp"
#include "src/Renderer/Renderer.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/Sprite/Common/SpriteCommon.hpp"
#include "src/Texture/TextureManager.h"
#include "src/Timer/Timer.hpp"

class IGame;

class Framework {
    GameEngine::Config* config_ = nullptr;

    std::unique_ptr<IGame> game_;

    std::unique_ptr<WinApp> windows_;
    std::unique_ptr<DirectXAdapter> dxAdaptor_;
    std::unique_ptr<SRVManager> srv_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Timer> timer_;

    Input* input_ = nullptr;
    TextureManager* texture_ = nullptr;
    SpriteCommon* sprite_ = nullptr;

public:
    Framework();

    void Execute(std::unique_ptr<IGame> _game);

private:
    void Initialize();
    bool Loop() const;
    void Update() const;
    void Draw() const;
    void Shutdown();
}; // class Framework

#endif // Framework_HPP_
