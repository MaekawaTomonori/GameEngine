#ifndef PipelineStateObject_HPP_
#define PipelineStateObject_HPP_
#include <d3d12.h>
#include <wrl/client.h>

#include "InputLayout.hpp"
#include "RootSignature.hpp"
#include "src/DirectX/DirectXAdapter.hpp"

class PipelineStateObject {
    DirectXAdapter* adapter_ = nullptr;

    RootSignature rootSignature_;
    InputLayout inputLayout_;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_;

public:
    PipelineStateObject() = delete;
    PipelineStateObject(DirectXAdapter* _adapter);
    PipelineStateObject SetRootSignature(RootSignature _rootSignature);
    PipelineStateObject SetInputLayout(InputLayout _inputLayout);

    void Create();
    

}; // class PipelineStateObject

#endif // PipelineStateObject_HPP_
