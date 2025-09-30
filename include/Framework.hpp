#ifndef Framework_HPP_
#define Framework_HPP_

#include <memory>

#include "DebugUI.hpp"
#include "Input.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "src/Camera/Director/CameraDirector.hpp"
#include "src/Platform/WinApp.hpp"
#include "src/Config/Config.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/Light/LightManager.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/Scene/SceneSwitcher.hpp"
#include "src/Sprite/Common/SpriteCommon.hpp"
#include "src/Line/Common/LineCommon.hpp"
#include "src/PostProcess/Executor/PostProcessExecutor.hpp"
#include "src/Renderer/Renderer.hpp"
#include "src/Sky/Common/SkyCommon.hpp"
#include "src/Stage/LevelEditor.hpp"
#include "src/Texture/TextureManager.hpp"

class IGame;

class Framework {
    GameEngine::Config* config_ = nullptr;

    std::unique_ptr<IGame> game_;
    SceneSwitcher* scene_ = nullptr;

    std::unique_ptr<WinApp> windows_;
    std::unique_ptr<DirectXAdapter> dxAdapter_;
    std::unique_ptr<PostProcessExecutor> postProcessor_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<ResourceRepository> resources_;
    std::unique_ptr<DebugUI> debugUI_;
    std::unique_ptr<SRVManager> srv_;
    std::unique_ptr<LevelEditor> level_;

    Input* input_ = nullptr;
    TextureManager* texture_ = nullptr;
    SpriteCommon* sprite_ = nullptr;
    ModelCommon* model_ = nullptr;
    LineCommon* line_ = nullptr;
    SkyCommon* sky_ = nullptr;
    LightManager* light_ = nullptr;

    CameraController* camera_ = nullptr;
    CameraDirector* cameraDirector_ = nullptr;

public:
    Framework();
    ~Framework();

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
