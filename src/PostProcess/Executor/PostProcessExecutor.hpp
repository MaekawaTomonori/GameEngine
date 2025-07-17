#ifndef PostProcessExecutor_HPP_
#define PostProcessExecutor_HPP_
#include <vector>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/PostProcess/IPostEffect.hpp"

class PostProcessExecutor {
    DirectXAdapter* adapter_ = nullptr;

    std::vector<std::unique_ptr<IPostEffect>> effects_;

public:
    void Initialize(DirectXAdapter* _adapter);
    void Add(std::unique_ptr<IPostEffect> _effect);
    void Execute() const;

private:
    void ToSwapChain() const;
}; // class PostProcessExecutor

#endif // PostProcessExecutor_HPP_
