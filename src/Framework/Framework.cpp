#include "include/Framework.hpp"

#include "Log.hpp"
#include "IGame.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Random/RandomEngine.hpp"

Framework::Framework() {
    Log::Initialize();

    // Log startup diagnostics
    Log::SendWithContext(Log::Level::INFO, "Framework initialization started", "FRAMEWORK");
    Log::LogFileOperation("STARTUP_CHECK", "Assets", std::filesystem::exists("Assets"), "Checking assets directory");
    Log::LogFileOperation("STARTUP_CHECK", "Assets/Shaders", std::filesystem::exists("Assets/Shaders"), "Checking shaders directory");

    windows_ = std::make_unique<WinApp>();
    windows_->Initialize();
    //windows_->SetWindowSize(static_cast<int>(config_->GetWidth()), static_cast<int>(config_->GetHeight()));

    dxAdapter_ = std::make_unique<DirectXAdapter>(windows_->GetWindowHandle(), config_.width, config_.height);

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

    particle_ = std::make_unique<ParticleSystem>(dxAdapter_.get(), srv_.get(), resources_->GetMeshRepository(), debugUI_.get());
    particle_->Initialize();

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
    camera_->Initialize(static_cast<float>(config_.width) / static_cast<float>(config_.height), debugUI_.get());

    cameraDirector_ = Singleton<CameraDirector>::GetInstance();
    cameraDirector_->Initialize(debugUI_.get());

    light_ = Singleton<LightManager>::GetInstance();
    light_->Initialize(dxAdapter_.get(), debugUI_.get());

    Singleton<RandomEngine>::GetInstance()->Initialize();

    //level_->Initialize("Level");
}

Framework::~Framework() {
    texture_->Unload();
    level_.reset();
    postProcessor_.reset();
    renderer_.reset();

    resources_.reset();
    debugUI_.reset();

    srv_->Finalize();
    srv_.reset();

    SingletonFinalizer::Finalize();

    CoUninitialize();
}

void Framework::Execute(std::unique_ptr<IGame> _game) {
    game_ = std::move(_game);
    Initialize();

    while (Loop()) {
        Update();
        Draw();
    }

    Shutdown();
}

void Framework::Initialize() {
    if (!game_) return;
    game_->Initialize(config_);
    windows_->SetTitle(config_.title);
    scene_ = game_->GetSceneSwitcher();
    scene_->Setup({ postProcessor_.get(), debugUI_.get(), particle_.get() });
    scene_->Change(config_.defaultScene);

    // PostEffectFactoryを設定
    if (const auto& peFac = game_->GetPostEffectFactory()) {
        postProcessor_->SetFactory(peFac);
    }

    windows_->SetTitle(config_.title);
    Log::Send(Log::Level::INFO, "Game Initialized");
}

bool Framework::Loop() const {
    if (!Check())return false;
    return windows_->IsEnabled();
}

void Framework::Update() const {
    if (!Check())return;

    input_->Update();

    /// Borderless fullscreen toggle
    if (input_->IsTrigger(DIK_F10)) {
        windows_->ToggleBorderless();

        int width, height;
        windows_->GetClientSize(width, height);
        HandleWindowResize(width, height);
    }

    cameraDirector_->Update();
    camera_->Update();
    light_->Update();
    level_->Update();
    particle_->Update();

    // PostEffect animation update
    float deltaTime = 1.0f / static_cast<float>(config_.fps);
    postProcessor_->Update(deltaTime);

    scene_->Update();
}

void Framework::Draw() const {
    if (!Check())return;

    dxAdapter_->DisplayFPS(debugUI_.get());
    postProcessor_->Debug();

    srv_->PreDraw();

    scene_->Draw();

    sky_->Draw(renderer_.get());
    model_->Draw(renderer_.get());
    particle_->Draw(renderer_.get());
    line_->Draw(renderer_.get());
    sprite_->Draw(renderer_.get());

    renderer_->Register([&] { level_->Draw(); });
    renderer_->Register([&] { debugUI_->Render(); });
    renderer_->Render();
}

void Framework::Shutdown() {
    if (game_) {
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

void Framework::HandleWindowResize(int _width, int _height) const {
    // DirectXリソースのリサイズ
    dxAdapter_->UpdateWindowSize(static_cast<size_t>(_width), static_cast<size_t>(_height));

    // ポストプロセッサのレンダーテクスチャをリサイズ
    postProcessor_->ResizeRenderTextures();

    // ImGuiのディスプレイサイズを更新
    debugUI_->UpdateDisplaySize(_width, _height);

    // カメラのアスペクト比を更新
    float aspectRatio = static_cast<float>(_width) / static_cast<float>(_height);
    if (camera_ && camera_->GetActive()) {
        camera_->GetActive()->SetAspectRatio(aspectRatio);
    }
}
