#include "Framework.hpp"

Framework::Framework(std::unique_ptr<IGame> _game) {
    game_ = std::move(_game);
}

void Framework::Run() {
    Initialize();

    do{
        Update();
    } while (engine_->IsEnabled());
}

void Framework::Initialize() {
    engine_ = std::make_unique<Engine>();
    engine_->Initialize();
    game_->Initialize();
}

void Framework::Update() {
}

void Framework::Shutdown() {
}
