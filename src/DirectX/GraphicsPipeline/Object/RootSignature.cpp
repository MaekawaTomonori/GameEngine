#include "RootSignature.hpp"

#include "Log.hpp"
#include "Utils.hpp"

RootSignature RootSignature::SetParameter(D3D12_ROOT_PARAMETER _parameter) {
    parameters_.push_back(_parameter);
    return *this;
}

RootSignature RootSignature::SetSampler(D3D12_STATIC_SAMPLER_DESC _sampler) {
    samplers_.push_back(_sampler);
    return *this;
}

void RootSignature::Create(ID3D12Device* _device) {
    if (!_device) {
        Log::Send(Log::Level::ERR, "Device is null");
        Utils::Alert("Device is null");
        return;
    }

    desc_.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    desc_.pParameters = parameters_.data();
    desc_.NumParameters = static_cast<UINT>(parameters_.size());
    desc_.pStaticSamplers = samplers_.data();
    desc_.NumStaticSamplers = static_cast<UINT>(samplers_.size());

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errBlob;

    HRESULT hr = D3D12SerializeRootSignature(
        &desc_,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &sigBlob,
        &errBlob
    );

    if (FAILED(hr)) {
        Log::Send(Log::Level::ERR, "Failed to serialize root signature");
        Utils::Alert("Failed to serialize root signature");
        return;
    }

    hr = _device->CreateRootSignature(
        0,
        sigBlob->GetBufferPointer(),
        sigBlob->GetBufferSize(),
        IID_PPV_ARGS(&data_)
    );

    if (FAILED(hr)) {
        Log::Send(Log::Level::ERR, "Failed to create root signature");
        Utils::Alert("Failed to create root signature");
        return;
    }

    Log::Send(Log::Level::INFO, "Root signature created successfully");
}

ID3D12RootSignature* RootSignature::Get() const {
    return data_.Get();
}
