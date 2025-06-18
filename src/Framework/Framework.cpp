#include "include/Framework.hpp"

#include "Log.hpp"
#include "Singleton.hpp"
#include "Utils.hpp"
#include "include/IGame.hpp"

Framework::Framework() {
    config_ = GameEngine::Config::Default();

    windows_ = std::make_unique<WinApp>();
    windows_->Initialize();
    //windows_->SetWindowSize(static_cast<int>(config_->GetWidth()), static_cast<int>(config_->GetHeight()));

    dxAdaptor_ = std::make_unique<DirectXAdapter>(windows_->GetWindowHandle(), config_->GetWidth(), config_->GetHeight());

    srv_ = std::make_unique<SRVManager>();
    srv_->Initialize(dxAdaptor_.get());

    renderer_ = std::make_unique<Renderer>();
    renderer_->Initialize(dxAdaptor_.get());

    input_ = Singleton<Input>::GetInstance();
    input_->Initialize();

    texture_ = Singleton<TextureManager>::GetInstance();
    texture_->Initialize(dxAdaptor_.get(), srv_.get());

    sprite_ = Singleton<SpriteCommon>::GetInstance();
    sprite_->Initialize(dxAdaptor_.get());

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
    game_->Initialize();
    config_ = &game_->GetCurrentConfig();
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
    game_->Update();
}

void Framework::Draw() const {
    if (!game_)return;
    game_->Draw();

    if (!dxAdaptor_){
        Utils::Alert("DirectXAdapter is not initialized");
    }
    //renderer_->Register([&](){debugUI_->Render(); });
    renderer_->Render();
}

void Framework::Shutdown() {
    if (game_){
        game_->Shutdown();
        game_.reset();
    }
    
    if (renderer_){
        renderer_->Shutdown();
        renderer_.reset();
    }

    SingletonFinalizer::Finalize();
    CoUninitialize();
}
