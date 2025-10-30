#ifndef Config_HPP_
#define Config_HPP_
#include <cstdint>
#include <memory>
#include <string>

namespace GameEngine {
    class Config{
        uint16_t fps_ = 60;
        std::string title_ = "Engine";
        size_t width_ = 800;
        size_t height_ = 600;

        static std::unique_ptr<Config> default_;
    public:
        Config();
        Config& SetFPS(uint16_t _fps);
        Config& SetTitle(const std::string& _title);
        Config& SetWidth(size_t _width);
        Config& SetHeight(size_t _height);
        Config& SetWindowSize(size_t _width, size_t _height);

        Config& Get();

        uint16_t GetFPS() const;

        size_t GetWidth() const;

        size_t GetHeight() const;

        std::string GetTitle() const;

        static Config* Default();
    };
}

#endif // Config_HPP_
