#include "include/Framework.hpp"

#include "Utils.hpp"
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

    sprite_ = Singleton<SpriteCommon>::GetInstance();
    sprite_->Initialize(dxAdapter_.get());

    timer_ = std::make_unique<Timer>(static_cast<std::chrono::milliseconds>(static_cast<uint64_t>(1e4 / 60)));
    timer_->Start();
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
    if (!game_)return;
    config_ = &game_->GetCurrentConfig();
    scene_ = game_->GetSceneSwitcher();
    Log::Send(Log::Level::INFO, "Game Initialized");
}

bool Framework::Loop() const {
    if (!windows_)return false;
    return windows_->IsEnabled();
}

void Framework::Update() const {
    if (input_)input_->Update();
    if (timer_->Check()) {
        timer_->Restart();
        return;
    }
    if (!game_)return;
    if (!scene_)return;
    scene_->Update();
}

void Framework::Draw() const {
    if (!game_)return;
    if (!scene_)return;
    if (debugUI_)debugUI_->Process();
    dxAdapter_->Register([&] { scene_->Draw(); });

    if (!dxAdapter_){
        Utils::Alert("DirectXAdapter is not initialized");
    }
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
