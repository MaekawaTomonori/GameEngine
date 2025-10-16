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
    virtual std::unique_ptr<IScene> Create(const std::string& sceneName) = 0;
}; // class AbstractSceneFactory

#endif // AbstractSceneFactory_HPP_
