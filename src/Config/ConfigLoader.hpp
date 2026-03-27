#ifndef ConfigLoader_HPP_
#define ConfigLoader_HPP_

#include <string>
#include "Config.hpp"

namespace GameEngine {
    /** @brief cnfファイルからConfigを読み込むユーティリティ
     *
     *  ファイル形式（key=value、# でコメント）:
     *  @code
     *  # Application Configuration
     *  fps=60
     *  width=1280
     *  height=720
     *  title=MyGame
     *  showCursor=false
     *  @endcode
     *
     *  ファイルが存在しない、または読み込めない場合は何もしない。
     *  ゲーム側（IGame::Initialize）でさらに上書きすることも可能。
     */
    class ConfigLoader {
    public:
        /** @brief cnfファイルを読み込み _config に反映する
         *  @param _path cnfファイルのパス（例: "Assets/Config/App.cnf"）
         *  @param _config 書き込み先のConfig（存在するキーのみ上書き）
         */
        static void Load(const std::string& _path, Config& _config);
    };
}

#endif // ConfigLoader_HPP_
