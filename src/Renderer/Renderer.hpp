#ifndef Renderer_HPP_
#define Renderer_HPP_

#include <functional>
#include <queue>
#include <string>

#include "ReferencePtr.hpp"

class DirectXAdapter;
class PostProcessExecutor;

/** @brief レンダラークラス
 * 描画タスクの管理とポストプロセス適用を制御
 */
class Renderer {
    GESTD::ReferencePtr<DirectXAdapter> adapter_ = nullptr;
    GESTD::ReferencePtr<PostProcessExecutor> postProcessor_ = nullptr;

    /** UIタスク（常にスワップチェーンへ描画：ImGui等）
     */
    std::queue<std::function<void()>> uiTasks_;

public:
    /** @brief レンダラーを初期化
     * @param _adapter DirectXアダプター
     * @param _postProcessor ポストプロセス実行管理
     */
    void Initialize(GESTD::ReferencePtr<DirectXAdapter> _adapter, GESTD::ReferencePtr<PostProcessExecutor> _postProcessor);

    /** @brief 描画タスクを登録
     * @param _task 描画タスク
     * @param _canvasName 登録先Canvas名
     */
    void Register(const std::function<void()>& _task, const std::string& _canvasName = "None");

    /** @brief UIタスクを登録（常にスワップチェーンへ描画）
     * @param _task 描画タスク（ImGui等）
     */
    void RegisterUI(const std::function<void()>& _task);

    /** @brief 登録されたタスクをレンダリング
     */
    void Render();

private:
}; // class Renderer

#endif // Renderer_HPP_
