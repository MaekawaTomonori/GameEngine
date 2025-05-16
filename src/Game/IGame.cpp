#include "include/IGame.hpp"

#include <utility>

IGame::Config::Config(std::string _title, uint16_t _fps) {
    title_ = std::move(_title);
    fps_ = _fps;
}

IGame::IGame() = default;

IGame::Config &IGame::SetConfig(const Config &_config) {
	config_ = _config;
	return config_;
}

IGame::Config& IGame::GetConfig() {
	return config_;
}
