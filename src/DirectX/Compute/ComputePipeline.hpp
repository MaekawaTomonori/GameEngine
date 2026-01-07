#ifndef ComputePipeline_HPP_
#define ComputePipeline_HPP_
#include <d3d12.h>
#include <dxcapi.h>
#include <string>
#include <wrl/client.h>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/RootSignature/RootSignature.hpp"

class ComputePipeline {
    std::optional<std::reference_wrapper<DirectXAdapter>> adapter_;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> state_;

    Microsoft::WRL::ComPtr<IDxcBlob> shader_;

    RootSignature rootSignature_;

public:
    ComputePipeline(std::reference_wrapper<DirectXAdapter> _adapter) :adapter_(_adapter) {}
    ComputePipeline& SetRootSignature(const RootSignature& _rootSignature);
    void Create(const std::string& _name);

private:

}; // class ComputePipeline

#endif // ComputePipeline_HPP_
