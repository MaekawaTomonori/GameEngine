#ifndef Renderer_HPP_
#define Renderer_HPP_

#include <functional>
#include <queue>

class DirectXAdapter;
class PostProcessExecutor;

class Renderer {
    DirectXAdapter* adapter_ = nullptr;
    PostProcessExecutor* postProcessor_ = nullptr;

    // PostProcessを適応するタスク
    std::queue<std::function<void()>> pp_;
    // PostProcessを適用しないタスク
    std::queue<std::function<void()>> tasks_;

public:
    void Initialize(DirectXAdapter* _adapter, PostProcessExecutor* _postProcessor);
    
    void Register(const std::function<void()>& _task, bool _applyPostEffect = false);
    
    void Render();
    
private:
}; // class Renderer

#endif // Renderer_HPP_