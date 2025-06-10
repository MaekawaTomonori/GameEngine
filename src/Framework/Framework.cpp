#include "include/Framework.hpp"
#include "include/IGame.hpp"

Framework::Framework() {
    engine_ = std::make_unique<Engine>();
    engine_->Initialize();
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
    engine_->ApplyConfig(config_);
}

bool Framework::Loop() const {
    if (!engine_)return false;
    if (!engine_->IsEnabled())return false;
    return true;
}

void Framework::Update() const {
    if (!engine_)return;
    engine_->Update();
    //if (engine_->UpdateSkip())return;
    if (!game_)return;
    game_->Update();
}

void Framework::Draw() const {
    if (!game_)return;
    game_->Draw();

    if (!engine_)return;
    engine_->Render();
}

void Framework::Shutdown() {
    if (game_){
        game_->Shutdown();
        game_.reset();
    }
    if (!engine_)return;
    engine_->Shutdown();
    engine_.reset();
}
