#ifndef SceneSwitcher_HPP_
#define SceneSwitcher_HPP_
#include <memory>
#include <string>

#include "DebugUI.hpp"
#include "IScene.hpp"
#include "Factory/AbstractSceneFactory.hpp"

class Transition;

/// <summary>
/// シーン切り替え管理クラス
/// シーン遷移とトランジションエフェクトを管理
/// </summary>
class SceneSwitcher {
	std::unique_ptr<AbstractSceneFactory> factory_;

    PostProcessExecutor* ppe_ = nullptr;
    DebugUI* debug_ = nullptr;

    std::unique_ptr<IScene> scene_;
    std::unique_ptr<IScene> next_;

    std::unique_ptr<Transition> transition_;

public:
    /// <summary>
    /// セットアップ
    /// </summary>
    /// <param name="_ppe">ポストプロセス実行管理</param>
    /// <param name="_debug">デバッグUI</param>
    void Setup(PostProcessExecutor* _ppe, DebugUI* _debug);

    /// <summary>
    /// 更新処理
    /// </summary>
	void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

    /// <summary>
    /// シーンファクトリーを設定
    /// </summary>
    /// <param name="_factory">シーンファクトリー</param>
    void SetFactory(std::unique_ptr<AbstractSceneFactory> _factory);

    /// <summary>
    /// シーンを変更
    /// </summary>
    /// <param name="_name">シーン名</param>
    void Change(const std::string& _name);

    /// <summary>
    /// デバッグ情報の表示
    /// </summary>
    void Debug();
}; // class SceneSwitcher

#endif // SceneSwitcher_HPP_
