#ifndef PostProcessExecutor_HPP_
#define PostProcessExecutor_HPP_
#include <vector>
#include <wrl/client.h>
#include <d3d12.h>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/PostProcess/IPostEffect.hpp"

class PostProcessExecutor {
    DirectXAdapter* adapter_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> rtr_;

    std::vector<IPostEffect*> effects_;
public:
    void Initialize(DirectXAdapter* _adapter);
    void Add(IPostEffect* _effect);
    void Execute() const;
}; // class PostProcessExecutor

#endif // PostProcessExecutor_HPP_
