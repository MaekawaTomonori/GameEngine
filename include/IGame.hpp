#ifndef IGame_HPP_
#define IGame_HPP_

#include <memory>

#include "src/Scene/SceneSwitcher.hpp"
#include "src/Config/Config.hpp"

class AbstractSceneFactory;
class PostProcessExecutor;

/// <summary>
/// ゲーム実装の基底インターフェース
/// シーン管理と設定の統合を提供
/// </summary>
class IGame {
    std::unique_ptr<GameEngine::Config> config_;
    std::unique_ptr<SceneSwitcher> scene_;

public:
    IGame(std::unique_ptr<AbstractSceneFactory> _factory, const std::string &_scene = "");
    virtual ~IGame() = default;

    /// <summary>
    /// 現在の設定を取得
    /// </summary>
    /// <returns>設定への参照</returns>
    GameEngine::Config& GetCurrentConfig();

    /// <summary>
    /// シーン切り替え管理を取得
    /// </summary>
    /// <returns>シーン切り替え管理のポインタ</returns>
    SceneSwitcher* GetSceneSwitcher() const;
}; // class IGame

#endif // IGame_HPP_
