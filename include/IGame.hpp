#ifndef IGame_HPP_
#define IGame_HPP_

#include <memory>

#include "ReferencePtr.hpp"
#include "src/Scene/SceneSwitcher.hpp"
#include "src/Config/Config.hpp"
#include "Factory/PostEffectFactory.hpp"
#include "src/PostProcess/IPostEffect.hpp"

class AbstractSceneFactory;
class PostProcessExecutor;

/** @brief ゲーム実装の基底インターフェース
 * シーン管理と設定の統合を提供
 */
class IGame {
    std::unique_ptr<SceneSwitcher> scene_;
    std::unique_ptr<PostEffectFactory> postEffectFactory_;

public:
    IGame();
    virtual ~IGame();

    virtual void Initialize(GameEngine::Config& _config) = 0;

    /** @brief シーン切り替え管理を取得
     * @return シーン切り替え管理のポインタ
     */
    GESTD::ReferencePtr<SceneSwitcher> GetSceneSwitcher() const;

    /** @brief PostEffectFactoryを取得
     * @return PostEffectファクトリーのポインタ
     */
    GESTD::ReferencePtr<PostEffectFactory> GetPostEffectFactory() const;

protected:
    template<typename T>
    void RegisterScene(const std::string& _name) {
        static_assert(std::is_base_of_v<IScene, T>, "T must be derived from IScene");
        scene_->RegisterScene(_name, [] { return std::make_unique<T>(); });
    }

    /** @brief PostEffectを登録する
     * @tparam T IPostEffectを継承したエフェクトクラス
     * @param _type エフェクトタイプ名
     */
    template<typename T>
    void RegisterPostEffect(const std::string& _type) {
        static_assert(std::is_base_of_v<IPostEffect, T>, "T must be derived from IPostEffect");
        postEffectFactory_->Register(_type, [] { return std::make_unique<T>(); });
    }
}; // class IGame

#endif // IGame_HPP_
