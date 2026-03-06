#ifndef AbstractSceneFactory_HPP_
#define AbstractSceneFactory_HPP_
#include <memory>
#include <string>

class IScene;

/** @brief シーンファクトリーの抽象基底クラス
 * ゲーム固有のシーン生成を実装するためのインターフェース
 */
class AbstractSceneFactory {
public:
	virtual ~AbstractSceneFactory() = default;

    /** @brief シーンを生成（純粋仮想関数）
     * @param sceneName シーン名
     * @return 生成されたシーンのユニークポインタ
     */
    virtual std::unique_ptr<IScene> Create(const std::string& _sceneName) = 0;
}; // class AbstractSceneFactory

#endif // AbstractSceneFactory_HPP_
