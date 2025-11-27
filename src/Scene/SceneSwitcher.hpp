#ifndef SceneSwitcher_HPP_
#define SceneSwitcher_HPP_
#include <memory>
#include <string>

#include "DebugUI.hpp"
#include "src/PostProcess/Editor/PostProcessPresetEditor.hpp"
#include "src/ParticleSystem/ParticleSystem.hpp"

#include "IScene.hpp"
#include "SceneFactory.hpp"

class Transition;

/// <summary>
/// シーン切り替え管理クラス
/// シーン遷移とトランジションエフェクトを管理
/// </summary>
class SceneSwitcher {
public:
    struct Context {
        PostProcessExecutor* ppe = nullptr;
        DebugUI* debug = nullptr;
        ParticleSystem* particle = nullptr;
    };

private:
    std::unique_ptr<SceneFactory> factory_;

    Context context_;

    std::unique_ptr<IScene> scene_;
    std::unique_ptr<IScene> next_;

    std::unique_ptr<Transition> transition_;

public:
    SceneSwitcher();
    /// <summary>
    /// セットアップ
    /// </summary>
    /// <param name="_context">コンテキスト</param>
    void Setup(const Context& _context);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

    /// @brief シーンを登録する
    /// @param _name シーン名
    /// @param _creator シーン生成関数
    void RegisterScene(const std::string& _name, const std::function<std::unique_ptr<IScene>()>& _creator) const;

    /// <summary>
    /// シーンを変更
    /// </summary>
    /// <param name="_name">シーン名</param>
    void Change(const std::string& _name);

    /// <summary>
    /// デバッグ情報の表示
    /// </summary>
    void Debug();

    const Context& GetContext() const;
}; // class SceneSwitcher

#endif // SceneSwitcher_HPP_
