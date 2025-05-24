#ifndef Config_HPP_
#define Config_HPP_
#include <cstdint>
#include <string>

namespace GameEngine {
	class Config{
		uint16_t fps_ = 60;
		std::string title_ = "Engine";
		size_t width_ = 800;
		size_t height_ = 600;
	public:
		Config() = default;
		Config& SetFPS(uint16_t _fps);
		Config& SetTitle(const std::string& _title);
		Config& SetWidth(size_t _width);
		Config& SetHeight(size_t _height);
		Config& SetWindowSize(size_t _width, size_t _height);

		Config& Get();
	};
}

#endif // Config_HPP_
