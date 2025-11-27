#ifndef IGame_HPP_
#define IGame_HPP_

#include <memory>

#include "src/Scene/SceneSwitcher.hpp"
#include "src/Config/Config.hpp"

class AbstractSceneFactory;
class AbstractPostEffectFactory;
class PostProcessExecutor;

/// <summary>
/// ゲーム実装の基底インターフェース
/// シーン管理と設定の統合を提供
/// </summary>
class IGame {
    std::unique_ptr<SceneSwitcher> scene_;
    std::unique_ptr<AbstractPostEffectFactory> postEffectFactory_;

public:
    IGame();
    virtual ~IGame();

    virtual void Initialize(GameEngine::Config& _config) = 0;

    /// <summary>
    /// シーン切り替え管理を取得
    /// </summary>
    /// <returns>シーン切り替え管理のポインタ</returns>
    SceneSwitcher* GetSceneSwitcher() const;

    /// <summary>
    /// PostEffectFactoryを設定
    /// </summary>
    /// <param name="_factory">PostEffectファクトリー</param>
    void SetPostEffectFactory(std::unique_ptr<AbstractPostEffectFactory> _factory);

    /// <summary>
    /// PostEffectFactoryを取得
    /// </summary>
    /// <returns>PostEffectファクトリーのポインタ</returns>
    AbstractPostEffectFactory* GetPostEffectFactory() const;

protected:
    void RegisterScene(const std::string& _name, const std::function<std::unique_ptr<IScene>()>& _creator) const;

}; // class IGame

#endif // IGame_HPP_
