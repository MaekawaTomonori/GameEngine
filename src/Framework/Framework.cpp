#include "include/Framework.hpp"

#include "Log.hpp"
#include "Singleton.hpp"
#include "include/IGame.hpp"

Framework::Framework() {
    config_ = GameEngine::Config::Default();

    windows_ = std::make_unique<WinApp>();
    windows_->Initialize();
    //windows_->SetWindowSize(static_cast<int>(config_->GetWidth()), static_cast<int>(config_->GetHeight()));

    dxAdapter_ = std::make_unique<DirectXAdapter>(windows_->GetWindowHandle(), config_->GetWidth(), config_->GetHeight());

    debugUI_ = std::make_unique<DebugUI>();
    debugUI_->Initialize(dxAdapter_.get());

    srv_ = std::make_unique<SRVManager>();
    srv_->Initialize(dxAdapter_.get());

    input_ = Singleton<Input>::GetInstance();
    input_->Initialize();

    texture_ = Singleton<TextureManager>::GetInstance();
    texture_->Initialize(dxAdapter_.get(), srv_.get());

    mesh_ = Singleton<MeshManager>::GetInstance();
    mesh_->Initialize(dxAdapter_.get());

    sprite_ = Singleton<SpriteCommon>::GetInstance();
    sprite_->Initialize(dxAdapter_.get(), debugUI_.get());

    model_ = Singleton<ModelCommon>::GetInstance();
    model_->Initialize(dxAdapter_.get(), debugUI_.get());

    camera_ = Singleton<CameraManager>::GetInstance();
    camera_->Initialize(static_cast<float>(config_->GetWidth()) / static_cast<float>(config_->GetHeight()), debugUI_.get());

    light_ = Singleton<LightManager>::GetInstance();
    light_->Initialize(dxAdapter_.get(), debugUI_.get());
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
    Log::Send(Log::Level::INFO, "Game Initialized");
}

bool Framework::Loop() const {
    if (!Check())return false;
    return windows_->IsEnabled();
}

void Framework::Update() const {
    if (!Check())return;

    input_->Update();
    camera_->Update();
    light_->Update();
    scene_->Update();
}

void Framework::Draw() const {
    if (!Check())return;

    srv_->PreDraw();

    dxAdapter_->Register([&] { scene_->Draw(); });
    dxAdapter_->Register([&](){debugUI_->Render(); });
    dxAdapter_->Render();
}

void Framework::Shutdown() {
    if (game_){
        game_.reset();
    }
    
    if (dxAdapter_){
        dxAdapter_.reset();
    }

    SingletonFinalizer::Finalize();
    CoUninitialize();
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
