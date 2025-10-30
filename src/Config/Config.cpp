#include "Config.hpp"

namespace GameEngine {

    std::unique_ptr<Config> Config::default_ = nullptr;

    Config::Config() = default;

    size_t Config::GetWidth() const {
        return width_;
    }

    size_t Config::GetHeight() const {
        return height_;
    }

    std::string Config::GetTitle() const {
        return title_;
    }

    Config* Config::Default() {
        if (default_){
            return default_.get();
        }

        default_ = std::make_unique<Config>();
        default_->SetFPS(60)
            .SetTitle("Engine")
            .SetWidth(1280)
            .SetHeight(720);
        return default_.get();
    }

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

    uint16_t Config::GetFPS() const {
        return fps_;
    }
}
