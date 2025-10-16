#ifndef Renderer_HPP_
#define Renderer_HPP_

#include <functional>
#include <queue>

class DirectXAdapter;
class PostProcessExecutor;

/// <summary>
/// レンダラークラス
/// 描画タスクの管理とポストプロセス適用を制御
/// </summary>
class Renderer {
    DirectXAdapter* adapter_ = nullptr;
    PostProcessExecutor* postProcessor_ = nullptr;

    // PostProcessを適応するタスク
    std::queue<std::function<void()>> pp_;
    // PostProcessを適用しないタスク
    std::queue<std::function<void()>> tasks_;

public:
    /// <summary>
    /// レンダラーを初期化
    /// </summary>
    /// <param name="_adapter">DirectXアダプター</param>
    /// <param name="_postProcessor">ポストプロセス実行管理</param>
    void Initialize(DirectXAdapter* _adapter, PostProcessExecutor* _postProcessor);

    /// <summary>
    /// 描画タスクを登録
    /// </summary>
    /// <param name="_task">描画タスク</param>
    /// <param name="_applyPostEffect">ポストエフェクトを適用するか</param>
    void Register(const std::function<void()>& _task, bool _applyPostEffect = false);

    /// <summary>
    /// 登録されたタスクをレンダリング
    /// </summary>
    void Render();
    
private:
}; // class Renderer

#endif // Renderer_HPP_