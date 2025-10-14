#include "include/Framework.hpp"

#include "Log.hpp"
#include "IGame.hpp"
#include "Pattern/Singleton.hpp"
#include "src/PostProcess/BoxBlur/BoxBlur.hpp"
#include "src/PostProcess/Grayscale/Grayscale.hpp"
#include "src/PostProcess/Vignette/Vignette.hpp"

Framework::Framework() {
    config_ = GameEngine::Config::Default();

    Log::Initialize();
    
    // Log startup diagnostics
    Log::SendWithContext(Log::Level::INFO, "Framework initialization started", "FRAMEWORK");
    Log::LogFileOperation("STARTUP_CHECK", "Assets", std::filesystem::exists("Assets"), "Checking assets directory");
    Log::LogFileOperation("STARTUP_CHECK", "Assets/Shaders", std::filesystem::exists("Assets/Shaders"), "Checking shaders directory");

    windows_ = std::make_unique<WinApp>();
    windows_->Initialize();
    //windows_->SetWindowSize(static_cast<int>(config_->GetWidth()), static_cast<int>(config_->GetHeight()));

    dxAdapter_ = std::make_unique<DirectXAdapter>(windows_->GetWindowHandle(), config_->GetWidth(), config_->GetHeight());

    srv_ = std::make_unique<SRVManager>();
    srv_->Initialize(dxAdapter_.get());

    debugUI_ = std::make_unique<DebugUI>();
    debugUI_->Initialize(dxAdapter_.get());

    postProcessor_ = std::make_unique<PostProcessExecutor>();
    postProcessor_->Initialize(dxAdapter_.get(), srv_.get(), debugUI_.get());

    renderer_ = std::make_unique<Renderer>();
    renderer_->Initialize(dxAdapter_.get(), postProcessor_.get());

    resources_ = std::make_unique<ResourceRepository>();
    resources_->Initialize();

    level_ = std::make_unique<LevelEditor>(debugUI_.get());

    input_ = Singleton<Input>::GetInstance();
    input_->Initialize(windows_->GetWindowHandle(), windows_->GetInstanceHandle());

    texture_ = Singleton<TextureManager>::GetInstance();
    texture_->Initialize(dxAdapter_.get(), srv_.get());

    sprite_ = Singleton<SpriteCommon>::GetInstance();
    sprite_->Initialize(dxAdapter_.get(), debugUI_.get());

    model_ = Singleton<ModelCommon>::GetInstance();
    model_->Initialize(dxAdapter_.get(), debugUI_.get(), resources_.get(), srv_.get());

    line_ = Singleton<LineCommon>::GetInstance();
    line_->Initialize(dxAdapter_.get(), debugUI_.get(), srv_.get());

    sky_ = Singleton<SkyCommon>::GetInstance();
    sky_->Initialize(dxAdapter_.get(), debugUI_.get());

    camera_ = Singleton<CameraController>::GetInstance();
    camera_->Initialize(static_cast<float>(config_->GetWidth()) / static_cast<float>(config_->GetHeight()), debugUI_.get());

    cameraDirector_ = Singleton<CameraDirector>::GetInstance();
    cameraDirector_->Initialize(debugUI_.get());

    light_ = Singleton<LightManager>::GetInstance();
    light_->Initialize(dxAdapter_.get(), debugUI_.get());

    //level_->Initialize("Level");
}

Framework::~Framework() {
    if (texture_) {
        texture_->Unload();
    }
    if (level_) {
        level_.reset();
    }

    if (postProcessor_) {
        postProcessor_.reset();
    }
    if (renderer_) {
        renderer_.reset();
    }
    if (resources_) {
        resources_.reset();
    }
    if (debugUI_) {
        debugUI_.reset();
    }
    if (srv_){
        srv_->Finalize();
        srv_.reset();
    }
    
    SingletonFinalizer::Finalize();
    
    CoUninitialize();
}

void Framework::Execute(std::unique_ptr<IGame> _game) {
    game_ = std::move(_game);
    Initialize();

    while (Loop()){
        Update();
        Draw();
    }

    Shutdown();
}

void Framework::Initialize() {
    if (!game_) return;
    config_ = &game_->GetCurrentConfig();
    scene_ = game_->GetSceneSwitcher();
    scene_->Setup(postProcessor_.get(), debugUI_.get());
    windows_->SetTitle(config_->GetTitle());
    Log::Send(Log::Level::INFO, "Game Initialized");
}

bool Framework::Loop() const {
    if (!Check())return false;
    return windows_->IsEnabled();
}

void Framework::Update() const {
    if (!Check())return;

    input_->Update();
    cameraDirector_->Update();
    camera_->Update();
    light_->Update();
    level_->Update();
    scene_->Update();
}

void Framework::Draw() const {
    if (!Check())return;

    dxAdapter_->DisplayFPS(debugUI_.get());
    postProcessor_->Debug();

    srv_->PreDraw();

    renderer_->Register([&] { scene_->Draw(); }, true);
    renderer_->Register([&]{ level_->Draw(); });
    renderer_->Register([&] { debugUI_->Render(); });
    renderer_->Render();
}

void Framework::Shutdown() {
    if (game_){
        game_.reset();
    }
}

bool Framework::Check() const {
    if (!game_)return false;
    if (!scene_)return false;
    if (!windows_)return false;
    if (!dxAdapter_)return false;
    if (!debugUI_)return false;
    if (!srv_)return false;
    if (!input_)return false;
    if (!texture_)return false;
    if (!sprite_)return false;

    return true;
}
