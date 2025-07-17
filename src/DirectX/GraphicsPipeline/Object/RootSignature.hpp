#ifndef RootSignature_HPP_
#define RootSignature_HPP_
#include <d3d12.h>
#include <vector>
#include <wrl/client.h>


class RootSignature {
    Microsoft::WRL::ComPtr<ID3D12RootSignature> data_;

    D3D12_ROOT_SIGNATURE_DESC desc_{};

    std::vector<D3D12_ROOT_PARAMETER> parameters_{};
    std::vector<D3D12_STATIC_SAMPLER_DESC> samplers_{};

public:
    RootSignature SetParameter(D3D12_ROOT_PARAMETER _parameter);
    RootSignature SetSampler(D3D12_STATIC_SAMPLER_DESC _sampler);
    void Create(ID3D12Device* _device);
    ID3D12RootSignature* Get() const;
}; // class RootSignature

#endif // RootSignature_HPP_
