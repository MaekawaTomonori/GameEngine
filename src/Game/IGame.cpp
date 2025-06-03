#include "include/IGame.hpp"

IGame::IGame() = default;

GameEngine::Config & IGame::GetCurrentConfig() {
    if (!config_) {
        config_ = std::make_unique<GameEngine::Config>();
    }
    return config_->Get();
}
