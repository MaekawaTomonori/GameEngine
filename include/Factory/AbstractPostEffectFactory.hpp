#ifndef AbstractPostEffectFactory_HPP_
#define AbstractPostEffectFactory_HPP_
#include <memory>
#include <string>

class IPostEffect;

/** @brief PostEffectファクトリーの抽象基底クラス
 ** ゲーム固有のPostEffect生成を実装するためのインターフェース
 **/
class AbstractPostEffectFactory {
public:
    virtual ~AbstractPostEffectFactory() = default;

    /** @brief PostEffectを生成（純粋仮想関数）
     ** @param type エフェクトタイプ名（例: "Vignette", "Bloom"）
     ** @return 生成されたPostEffectのユニークポインタ、未知のタイプの場合はnullptr
     **/
    virtual std::unique_ptr<IPostEffect> Create(const std::string& type) = 0;
}; // class AbstractPostEffectFactory

#endif // AbstractPostEffectFactory_HPP_
