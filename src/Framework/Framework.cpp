#include "include/Framework.hpp"

#include "Log.hpp"
#include "IGame.hpp"
#include "Pattern/Singleton.hpp"
#include "src/PostProcess/BoxBlur/BoxBlur.hpp"
#include "src/PostProcess/Grayscale/Grayscale.hpp"
#include "src/PostProcess/Vignette/Vignette.hpp"

Framework::Framework() {
    config_ = GameEngine::Config::Default();

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

    stageRepository_ = std::make_unique<StageRepository>();

    stageLoader_ = std::make_unique<StageLoader>();
    stageLoader_->Initialize(stageRepository_.get());

    input_ = Singleton<Input>::GetInstance();
    input_->Initialize();

    texture_ = Singleton<TextureManager>::GetInstance();
    texture_->Initialize(dxAdapter_.get(), srv_.get());

    sprite_ = Singleton<SpriteCommon>::GetInstance();
    sprite_->Initialize(dxAdapter_.get(), debugUI_.get());

    model_ = Singleton<ModelCommon>::GetInstance();
    model_->Initialize(dxAdapter_.get(), debugUI_.get(), resources_.get(), srv_.get());

    line_ = Singleton<LineCommon>::GetInstance();
    line_->Initialize(dxAdapter_.get(), debugUI_.get(), srv_.get());

    camera_ = Singleton<CameraManager>::GetInstance();
    camera_->Initialize(static_cast<float>(config_->GetWidth()) / static_cast<float>(config_->GetHeight()), debugUI_.get());

    light_ = Singleton<LightManager>::GetInstance();
    light_->Initialize(dxAdapter_.get(), debugUI_.get());

    postProcessor_->Add(std::make_unique<Grayscale>(dxAdapter_.get(), srv_.get()), "Grayscale");
    postProcessor_->Add(std::make_unique<Vignette>(dxAdapter_.get(), srv_.get()), "Vignette");
    postProcessor_->Add(std::make_unique<BoxBlur>(dxAdapter_.get(), srv_.get()), "BoxBlur");

    if (!stageLoader_->Load("Level")) {
        Log::Send(Log::Level::ERR, "Failed to load stage data");
    } else{
        Log::Send(Log::Level::INFO, "Stage data loaded successfully");
    }
}

Framework::~Framework() {
    if (texture_) {
        texture_->Unload();
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
    
    // Clear adapter last
    if (dxAdapter_) {
        dxAdapter_.reset();
    }
    if (windows_) {
        windows_.reset();
    }
    
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

    dxAdapter_->DisplayFPS(debugUI_.get());
    postProcessor_->Debug();

    srv_->PreDraw();

    renderer_->Register([&] { scene_->Draw(); }, true);
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
