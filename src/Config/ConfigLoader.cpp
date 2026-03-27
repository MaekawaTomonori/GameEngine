#include "ConfigLoader.hpp"

#include <fstream>
#include <map>
#include <string>

namespace GameEngine {
    void ConfigLoader::Load(const std::string& _path, Config& _config) {
        std::ifstream file(_path);
        if (!file.is_open()) return;

        std::map<std::string, std::string> params;
        std::string line;
        while (std::getline(file, line)) {
            // 空行 / コメント行をスキップ
            if (line.empty() || line[0] == '#') continue;
            const auto pos = line.find('=');
            if (pos == std::string::npos) continue;
            params[line.substr(0, pos)] = line.substr(pos + 1);
        }

        if (params.count("fps"))        _config.fps        = static_cast<uint16_t>(std::stoi (params.at("fps")));
        if (params.count("width"))      _config.width      = static_cast<size_t>  (std::stoul(params.at("width")));
        if (params.count("height"))     _config.height     = static_cast<size_t>  (std::stoul(params.at("height")));
        if (params.count("title"))      _config.title      = params.at("title");
        if (params.count("showCursor")) _config.showCursor = (params.at("showCursor") == "true");
    }
}
