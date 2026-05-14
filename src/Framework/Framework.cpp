#include "include/Framework.hpp"

#include "Log.hpp"
#include "IGame.hpp"
#include "PerformanceProfiler.hpp"
#include "Pattern/Singleton.hpp"
#include "Random/RandomEngine.hpp"
#include "src/Scene/Sample/SampleScene.hpp"
#include "src/Config/ConfigLoader.hpp"
#include "src/Screen/Screen.hpp"

namespace {
    constexpr const char* APP_CONFIG_PATH = "Assets/Config/App.cnf";
}

Framework::Framework() {
    Log::Initialize();

    // cnfファイルを先読みして、DirectX初期化前に解像度・FPS等を確定させる
    // ファイルが存在しない場合は Config のデフォルト値を維持する
    GameEngine::ConfigLoader::Load(APP_CONFIG_PATH, config_);
    Log::LogFileOperation("STARTUP_CHECK", APP_CONFIG_PATH,
        std::filesystem::exists(APP_CONFIG_PATH), "Loading app config");

    Log::SendWithContext(Log::Level::INFO, "Framework initialization started", "FRAMEWORK");
    Log::LogFileOperation("STARTUP_CHECK", "Assets",         std::filesystem::exists("Assets"),         "Checking assets directory");
    Log::LogFileOperation("STARTUP_CHECK", "Assets/Shaders", std::filesystem::exists("Assets/Shaders"), "Checking shaders directory");

    // AUDIO_DISABLED: Audio::Initialize();

    windows_ = std::make_unique<WinApp>();
    windows_->Initialize();
    windows_->SetWindowSize(static_cast<int>(config_.width), static_cast<int>(config_.height));

    dxAdapter_ = std::make_unique<DirectXAdapter>(windows_->GetWindowHandle(), config_.width, config_.height);
    dxAdapter_->Initialize();

    Singleton<Screen>::GetInstance()->Resize(
        static_cast<float>(config_.width),
        static_cast<float>(config_.height));

    srv_ = std::make_unique<SRVManager>();
    srv_->Initialize(dxAdapter_.get());

#ifdef _DEBUG
    debugger_ = std::make_unique<Debugger>();
    debugger_->Initialize(GESTD::ReferencePtr<DirectXAdapter>(dxAdapter_));
    const auto dbg = debugger_->GetUI();
#else
    const GESTD::ReferencePtr<DebugUI> dbg = nullptr;
#endif

    postProcessor_ = std::make_unique<PostProcessExecutor>();
    postProcessor_->Initialize(GESTD::ReferencePtr(dxAdapter_), GESTD::ReferencePtr(srv_), dbg);

    renderer_ = std::make_unique<Renderer>();
    renderer_->Initialize(GESTD::ReferencePtr(dxAdapter_), GESTD::ReferencePtr(postProcessor_));

    resources_ = std::make_unique<ResourceRepository>();
    resources_->Initialize();

    particle_ = std::make_unique<ParticleSystem>(GESTD::ReferencePtr(dxAdapter_), srv_.get(), resources_->GetMeshRepository(), dbg);
    particle_->Initialize();

    input_ = Singleton<Input>::GetInstance();
    input_->Initialize(windows_->GetWindowHandle(), windows_->GetInstanceHandle());

    texture_ = Singleton<TextureManager>::GetInstance();
    texture_->Initialize(dxAdapter_.get(), srv_.get());

    text_ = Singleton<TextCommon>::GetInstance();
    text_->Initialize(GESTD::ReferencePtr(dxAdapter_), dbg, static_cast<TextureManager*>(texture_), srv_.get());

    sprite_ = Singleton<SpriteCommon>::GetInstance();
    sprite_->Initialize(GESTD::ReferencePtr(dxAdapter_), dbg);

    model_ = Singleton<ModelCommon>::GetInstance();
    model_->Initialize(GESTD::ReferencePtr(dxAdapter_), dbg, GESTD::ReferencePtr<ResourceRepository>(resources_), srv_.get());

    line_ = Singleton<LineCommon>::GetInstance();
    line_->Initialize(GESTD::ReferencePtr(dxAdapter_), dbg, srv_.get());

    sky_ = Singleton<SkyCommon>::GetInstance();
    sky_->Initialize(GESTD::ReferencePtr(dxAdapter_), dbg);

    camera_ = Singleton<CameraController>::GetInstance();
    camera_->Initialize(static_cast<float>(config_.width) / static_cast<float>(config_.height), dbg);

    cameraDirector_ = Singleton<CameraDirector>::GetInstance();
    cameraDirector_->Initialize(dbg);

    light_ = Singleton<LightManager>::GetInstance();
    light_->Initialize(GESTD::ReferencePtr(dxAdapter_), dbg);

    shadowCubeMap_ = std::make_unique<ShadowCubeMap>();
    shadowCubeMap_->Initialize(dxAdapter_.get(), srv_.get());

    shadowPass_ = std::make_unique<ShadowPass>();
    shadowPass_->Initialize(dxAdapter_.get());

    Singleton<RandomEngine>::GetInstance()->Initialize();

    collision_ = std::make_unique<CollisionManager>();
    collision_->Initialize(dbg);

    ui_ = Singleton<Ui::Manager>::GetInstance();
    ui_->Setup(dbg, input_);

#ifdef _DEBUG
    Debugger::WatchGroup("Engine")
        .Watch("FPS", &config_.fps)
        .Watch("ShowCursor", &config_.showCursor);

    // AUDIO_DISABLED: audioPanel_ = std::make_unique<AudioDebugPanel>();
    // AUDIO_DISABLED: audioPanel_->Initialize(debugger_->GetUI());
#endif
}

Framework::~Framework() {
    // AUDIO_DISABLED: Audio::Shutdown();
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

    if (!scene_) {
        Utils::Alert("SceneSwitcher is Null");
    }

#ifdef _DEBUG
    scene_->RegisterScene("sample", []{return std::make_unique<SampleScene>(); });

    const auto dbg = debugger_->GetUI();
#else
    const GESTD::ReferencePtr<DebugUI> dbg = nullptr;
#endif

    SceneSwitcher::Context ctx{ GESTD::ReferencePtr<PostProcessExecutor>(postProcessor_), GESTD::ReferencePtr<ParticleSystem>(particle_), dbg };
#ifdef _DEBUG
    ctx.frame = debugger_->GetFrame();
#endif
    scene_->Setup(ctx);
    scene_->Change(config_.defaultScene);

#ifdef _DEBUG
    debugger_->SetStopCallback([this]() {
        if (scene_) scene_->Change(config_.defaultScene);
    });
#endif

    if (const auto& peFac = game_->GetPostEffectFactory()) {
        postProcessor_->SetFactory(peFac);
    }

    windows_->SetTitle(config_.title);
    input_->SetCursorVisible(config_.showCursor);
    Log::Send(Log::Level::INFO, "Game Initialized");
}

bool Framework::Loop() const {
    if (!Check()) return false;
    return windows_->IsEnabled();
}

void Framework::Update() const {
    if (!Check()) return;

    input_->Update();

    /// Borderless fullscreen toggle
    if (input_->IsTrigger(DIK_F10)) {
        windows_->ToggleBorderless();

        int width, height;
        windows_->GetClientSize(width, height);
        HandleWindowResize(width, height);
    }

#ifdef _DEBUG
    dxAdapter_->DisplayFPS(debugger_->GetUI());
    cameraDirector_->Debug();
    camera_->Debug();
    light_->Debug();
    particle_->Debug();
    postProcessor_->Debug();
    collision_->Debug();
    // AUDIO_DISABLED: audioPanel_->Debug();
    debugger_->Debug();
    model_->Debug();
    sprite_->Debug();
    ui_->Debug();
    text_->Debug();

    scene_->Debug();
    if (debugger_->ShouldUpdate()) {
#endif

        { PROFILE_SCOPE("CameraDirector"); cameraDirector_->Update(); }
        { PROFILE_SCOPE("Particle");       particle_->Update(); }

        const float deltaTime = 1.0f / static_cast<float>(config_.fps);
        postProcessor_->Update(deltaTime);

        { PROFILE_SCOPE("Scene");          scene_->Update(); }
        { PROFILE_SCOPE("Collision");      collision_->Update(); }

#ifdef _DEBUG
    }
#endif

    { PROFILE_SCOPE("Camera");  camera_->Update(); }
    { PROFILE_SCOPE("Light");   light_->Update(); }
    { PROFILE_SCOPE("Model");   model_->Update(); }
    { PROFILE_SCOPE("Sprite");  sprite_->Update(); }
    { PROFILE_SCOPE("UI");      ui_->Update(); }
    text_->Update();
}

void Framework::Draw() const {
    if (!Check()) return;

#ifdef _DEBUG
    Log::Debug(debugger_->GetUI());
#endif

    srv_->PreDraw();

#ifdef _DEBUG
    cameraDirector_->Draw();
#endif

    scene_->Draw();
    ui_->Draw();

    sky_->Draw(renderer_.get());

    if (shadowCubeMap_ && shadowPass_ && light_) {
        shadowPass_->Execute(*shadowCubeMap_, *model_, *light_);
        model_->SetShadowBinding(shadowCubeMap_->GetSRVIndex(), shadowPass_->GetShadowDataAddress());
    }

    model_->Draw(renderer_.get());
    particle_->Draw(renderer_.get());
    collision_->DrawDebug();
    line_->Draw(renderer_.get());
    sprite_->Draw(renderer_.get());
    text_->Draw(renderer_.get());

#ifdef _DEBUG
    renderer_->RegisterUI([&] { debugger_->Render(); });
#endif
    renderer_->Render();

    // SceneView の矩形は ImGui レンダリング後に確定するため、ここでカーソル表示を反映
#ifdef _DEBUG
    // DebugUI パネル上ではカーソルを強制表示（SceneView 上に重なった場合も含む）
    input_->SetDebugUIHovered(debugger_->GetUI()->IsMouseOverDebugUI());
#endif
    input_->ApplyCursorVisibility();
}

void Framework::Shutdown() {
    if (game_) {
        game_.reset();
    }

    shadowPass_.reset();
    shadowCubeMap_.reset();

    collision_.reset();
    text_.Reset();  // Singleton は SingletonFinalizer::Finalize() で破棄される
    texture_->Unload();
    particle_.reset();
    postProcessor_.reset();
    renderer_.reset();
    resources_.reset();

#ifdef _DEBUG
    debugger_.reset();
#endif

    srv_->Finalize();
    srv_.reset();
}

bool Framework::Check() const {
    if (!game_)    return false;
    if (!scene_)   return false;
    if (!windows_) return false;
    if (!dxAdapter_) return false;
#ifdef _DEBUG
    if (!debugger_) return false;
#endif
    if (!srv_)     return false;
    if (!input_)   return false;
    if (!texture_) return false;
    if (!sprite_)  return false;

    return true;
}

void Framework::HandleWindowResize(int _width, int _height) const {
    dxAdapter_->UpdateWindowSize(static_cast<size_t>(_width), static_cast<size_t>(_height));
    Singleton<Screen>::GetInstance()->Resize(
        static_cast<float>(_width),
        static_cast<float>(_height));
    postProcessor_->ResizeRenderTextures();

#ifdef _DEBUG
    debugger_->UpdateDisplaySize(_width, _height);
#endif

    float aspectRatio = static_cast<float>(_width) / static_cast<float>(_height);
    if (camera_ && camera_->GetActive()) {
        camera_->GetActive()->SetAspectRatio(aspectRatio);
    }
}
