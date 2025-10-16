#ifndef AbstractSceneFactory_HPP_
#define AbstractSceneFactory_HPP_
#include <memory>
#include <string>

class IScene;

/// <summary>
/// シーンファクトリーの抽象基底クラス
/// ゲーム固有のシーン生成を実装するためのインターフェース
/// </summary>
class AbstractSceneFactory {
public:
	virtual ~AbstractSceneFactory() = default;

    /// <summary>
    /// シーンを生成（純粋仮想関数）
    /// </summary>
    /// <param name="sceneName">シーン名</param>
    /// <returns>生成されたシーンのユニークポインタ</returns>
    virtual std::unique_ptr<IScene> Create(const std::string& sceneName) = 0;
}; // class AbstractSceneFactory

#endif // AbstractSceneFactory_HPP_
