#ifndef SceneFactory_HPP_
#define SceneFactory_HPP_
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class IScene;

/** @brief シーンファクトリーの抽象基底クラス
 ** ゲーム固有のシーン生成を実装するためのインターフェース
 **/
class SceneFactory {
    using CreateFunc = std::function<std::unique_ptr<IScene>()>;
    std::unordered_map<std::string, CreateFunc> creators_;

public:

    /// @brief シーンを登録する
    /// @param _name シーンの登録名
    /// @param _creator ISceneを継承したシーン生成関数
    void Register(const std::string& _name, const CreateFunc& _creator);

    /// @brief シーンを生成（純粋仮想関数）
    /// @param _name シーン名
    /// @returns 生成されたシーン
    std::unique_ptr<IScene> Create(const std::string& _name);

    /// @brief 登録されているシーン名の一覧を取得
    /// @returns シーン名のリスト
    std::vector<std::string> GetRegisteredScenes() const;
};

#endif // SceneFactory_HPP_
