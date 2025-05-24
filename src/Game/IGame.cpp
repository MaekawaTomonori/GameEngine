#include "include/IGame.hpp"

IGame::IGame() = default;

GameEngine::Config & IGame::GetCurrentConfig() const {
	return config_->Get();
}
