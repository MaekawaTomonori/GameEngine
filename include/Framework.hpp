#ifndef Framework_HPP_
#define Framework_HPP_

#include <memory>

#include "DebugUI.hpp"
#include "Input.hpp"
#include "src/Camera/Manager/CameraManager.hpp"
#include "src/Platform/WinApp.hpp"
#include "src/Config/Config.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/Mesh/Loader/MeshManager.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/Scene/SceneSwitcher.hpp"
#include "src/Sprite/Common/SpriteCommon.hpp"
#include "src/Texture/TextureManager.hpp"

class IGame;

class Framework {
    GameEngine::Config* config_ = nullptr;

    std::unique_ptr<IGame> game_;
    SceneSwitcher* scene_ = nullptr;

    std::unique_ptr<WinApp> windows_;
    std::unique_ptr<DirectXAdapter> dxAdapter_;
    std::unique_ptr<DebugUI> debugUI_;
    std::unique_ptr<SRVManager> srv_;

    Input* input_ = nullptr;
    TextureManager* texture_ = nullptr;
    MeshManager* mesh_ = nullptr;
    SpriteCommon* sprite_ = nullptr;
    ModelCommon* model_ = nullptr;
	CameraManager* camera_ = nullptr;

public:
    Framework();

    void Execute(std::unique_ptr<IGame> _game);

private:
    void Initialize();
    bool Loop() const;
    void Update() const;
    void Draw() const;
    void Shutdown();

    bool Check() const;
}; // class Framework

#endif // Framework_HPP_
