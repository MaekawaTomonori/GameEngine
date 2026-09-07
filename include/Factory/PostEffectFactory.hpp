#ifndef PostEffectFactory_HPP_
#define PostEffectFactory_HPP_
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class IPostEffect;

/** @brief PostEffectファクトリークラス
 * PostEffectの生成関数を登録し、型名から生成する。
 * SceneFactoryと同じRegisterパターンを採用し、ゲーム側に生成分岐を書かせない。
 */
class PostEffectFactory {
    using CreateFunc = std::function<std::unique_ptr<IPostEffect>()>;
    std::unordered_map<std::string, CreateFunc> creators_;

public:
    /** @brief PostEffectを登録する
     * @param _type エフェクトタイプ名（例: "Vignette"）
     * @param _creator IPostEffectを継承したエフェクト生成関数
     */
    void Register(const std::string& _type, const CreateFunc& _creator);

    /** @brief PostEffectを生成する
     * @param _type エフェクトタイプ名
     * @returns 生成されたPostEffect。未登録の場合はnullptr
     */
    std::unique_ptr<IPostEffect> Create(const std::string& _type);

    /** @brief 登録されているエフェクトタイプ名の一覧を取得
     * @returns エフェクトタイプ名のリスト
     */
    std::vector<std::string> GetRegisteredTypes() const;
}; // class PostEffectFactory

#endif // PostEffectFactory_HPP_
