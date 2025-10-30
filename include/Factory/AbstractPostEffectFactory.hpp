#ifndef AbstractPostEffectFactory_HPP_
#define AbstractPostEffectFactory_HPP_
#include <memory>
#include <string>

class IPostEffect;

/// <summary>
/// PostEffectファクトリーの抽象基底クラス
/// ゲーム固有のPostEffect生成を実装するためのインターフェース
/// </summary>
class AbstractPostEffectFactory {
public:
    virtual ~AbstractPostEffectFactory() = default;

    /// <summary>
    /// PostEffectを生成（純粋仮想関数）
    /// </summary>
    /// <param name="type">エフェクトタイプ名（例: "Vignette", "Bloom"）</param>
    /// <returns>生成されたPostEffectのユニークポインタ、未知のタイプの場合はnullptr</returns>
    virtual std::unique_ptr<IPostEffect> Create(const std::string& type) = 0;
}; // class AbstractPostEffectFactory

#endif // AbstractPostEffectFactory_HPP_
