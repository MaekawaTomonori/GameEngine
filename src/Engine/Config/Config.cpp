#include "Config.hpp"

namespace GameEngine {

	Config& Config::SetFPS(uint16_t _fps) {
		fps_ = _fps;
		return *this;
	}

	Config& Config::SetTitle(const std::string& _title) {
		title_ = _title;
		return *this;
	}

	Config& Config::SetWidth(size_t _width) {
		width_ = _width;
		return *this;
	}

	Config& Config::SetHeight(size_t _height) {
		height_ = _height;
		return *this;
	}

	Config& Config::SetWindowSize(size_t _width, size_t _height) {
		SetWidth(_width);
		SetHeight(_height);
		return *this;
	}

	Config & Config::Get() {
		return *this;
	}
}
