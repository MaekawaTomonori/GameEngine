#ifndef IGame_HPP_
#define IGame_HPP_

#include <memory>

#include "WeakPtr.hpp"
#include "src/Scene/SceneSwitcher.hpp"
#include "src/Config/Config.hpp"

class AbstractSceneFactory;
class AbstractPostEffectFactory;
class PostProcessExecutor;

/** @brief ゲーム実装の基底インターフェース
 * シーン管理と設定の統合を提供
 */
class IGame {
    std::unique_ptr<SceneSwitcher> scene_;
    std::unique_ptr<AbstractPostEffectFactory> postEffectFactory_;

public:
    IGame();
    virtual ~IGame();

    virtual void Initialize(GameEngine::Config& _config) = 0;

    /** @brief シーン切り替え管理を取得
     * @return シーン切り替え管理のポインタ
     */
    GESTD::WeakPtr<SceneSwitcher> GetSceneSwitcher() const;

    /** @brief PostEffectFactoryを取得
     * @return PostEffectファクトリーのポインタ
     */
    GESTD::WeakPtr<AbstractPostEffectFactory> GetPostEffectFactory() const;

protected:
    template<typename T>
    void RegisterScene(const std::string& _name) {
        static_assert(std::is_base_of_v<IScene, T>, "T must be derived from IScene");
        scene_->RegisterScene(_name, [] { return std::make_unique<T>(); });
    }

    /** @brief PostEffectFactoryを設定
     * @param _factory PostEffectファクトリー
     */
    void SetPostEffectFactory(std::unique_ptr<AbstractPostEffectFactory> _factory);
}; // class IGame

#endif // IGame_HPP_
