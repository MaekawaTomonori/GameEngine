#ifndef Config_HPP_
#define Config_HPP_
#include <cstdint>
#include <string>

namespace GameEngine {
    struct Config{
        std::string title = "Engine";
        std::string defaultScene = "sample";
        size_t width = 1280;
        size_t height = 720;
        uint16_t fps = 60;
    };
}

#endif // Config_HPP_
