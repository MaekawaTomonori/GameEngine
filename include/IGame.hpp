#ifndef IGame_HPP_
#define IGame_HPP_

#include <memory>

#include "src/Scene/SceneSwitcher.hpp"
#include "src/Config/Config.hpp"

class AbstractSceneFactory;
class AbstractPostEffectFactory;
class PostProcessExecutor;

/** @brief ゲーム実装の基底インターフェース
 ** シーン管理と設定の統合を提供
 **/
class IGame {
    std::unique_ptr<SceneSwitcher> scene_;
    std::unique_ptr<AbstractPostEffectFactory> postEffectFactory_;

public:
    IGame();
    virtual ~IGame();

    virtual void Initialize(GameEngine::Config& _config) = 0;

    /** @brief シーン切り替え管理を取得
     ** @return シーン切り替え管理のポインタ
     **/
    SceneSwitcher* GetSceneSwitcher() const;

    /** @brief PostEffectFactoryを設定
     ** @param _factory PostEffectファクトリー
     **/
    void SetPostEffectFactory(std::unique_ptr<AbstractPostEffectFactory> _factory);

    /** @brief PostEffectFactoryを取得
     ** @return PostEffectファクトリーのポインタ
     **/
    AbstractPostEffectFactory* GetPostEffectFactory() const;

protected:
    void RegisterScene(const std::string& _name, const std::function<std::unique_ptr<IScene>()>& _creator) const;

}; // class IGame

#endif // IGame_HPP_
