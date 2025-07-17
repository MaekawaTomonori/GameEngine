#ifndef Renderer_HPP_
#define Renderer_HPP_

#include <functional>
#include <queue>
#include <memory>

#include "RenderPhase.hpp"

class DirectXAdapter;
class PostProcessExecutor;

class Renderer {
    DirectXAdapter* adapter_ = nullptr;
    PostProcessExecutor* postProcessor_ = nullptr;
    
    // フェーズ別タスクキュー
    std::queue<std::function<void()>> sceneTasks_;
    std::queue<std::function<void()>> uiTasks_;
    
public:
    void Initialize(DirectXAdapter* _adapter, PostProcessExecutor* _postProcessor);
    
    // 統一されたRegisterメソッド
    void Register(std::function<void()> _task, RenderPhase _phase);
    
    // レンダリング実行
    void Render();
    
private:
    void RenderScene();
    void ApplyPostEffects();
    void RenderUI();
    
    void SetRenderTarget();
    void SetSceneRenderTarget();
    void SetSwapChainRenderTarget();
}; // class Renderer

#endif // Renderer_HPP_