#ifndef Renderer_HPP_
#define Renderer_HPP_

#include <functional>
#include <queue>

#include "ReferencePtr.hpp"

class DirectXAdapter;
class PostProcessExecutor;

/** @brief レンダラークラス
 * 描画タスクの管理とポストプロセス適用を制御
 */
class Renderer {
    GESTD::ReferencePtr<DirectXAdapter> adapter_ = nullptr;
    GESTD::ReferencePtr<PostProcessExecutor> postProcessor_ = nullptr;

    /** PostProcessを適応するタスク
     */
    std::queue<std::function<void()>> pp_;
    /** PostProcessを適用しないゲームオブジェクトのタスク
     */
    std::queue<std::function<void()>> tasks_;
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
     * @param _applyPostEffect ポストエフェクトを適用するか
     */
    void Register(const std::function<void()>& _task, bool _applyPostEffect = false);

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
