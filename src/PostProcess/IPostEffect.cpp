#include "IPostEffect.hpp"

#include <d3d12.h>
#include <filesystem>
#include <format>
#include <fstream>

#include "Log.hpp"
#include "Utils.hpp"

using json = nlohmann::json;

void IPostEffect::SetUp(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv) {
    adapter_ = _adapter;
    srv_ = _srv;
    CreateOutput();
}

D3D12_GPU_DESCRIPTOR_HANDLE IPostEffect::Apply(const D3D12_GPU_DESCRIPTOR_HANDLE _handle) {
    if (!output_) Utils::Alert("");

    output_->ChangeState(adapter_->GetCommandList(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    pso_->DrawCall();

    adapter_->GetCommandList()->OMSetRenderTargets(1, &rtvHandle_, false, nullptr);
    adapter_->GetCommandList()->ClearRenderTargetView(rtvHandle_, &CLEAR_COLOR.x, 0, nullptr);

    adapter_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(0, _handle);

    Modifier();

    adapter_->GetCommandList()->DrawInstanced(3, 1, 0, 0);

    output_->ChangeState(adapter_->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    return srv_->GetGPUHandle(index_);
}

void IPostEffect::CreateOutput() {
    output_ = adapter_->CreateRenderTextureResource(static_cast<uint32_t>(adapter_->GetWidth()), static_cast<uint32_t>(adapter_->GetHeight()), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, CLEAR_COLOR);

    index_ = srv_->Allocate();
    srv_->CreateSRVForTexture2D(index_, output_->Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);
    handle_ = srv_->GetGPUHandle(index_);
}

void IPostEffect::SetRTVHandle(D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle) {
    rtvHandle_ = _rtvHandle;

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    adapter_->GetDevice()->CreateRenderTargetView(output_->Get(), &rtvDesc, rtvHandle_);
}

std::string IPostEffect::BuildPresetPath(const std::string& _presetName) const {
    return "./Assets/Data/PostEffect/" + GetTypeName() + "/" + _presetName + ".json";
}

bool IPostEffect::LoadKeyframeFile(const std::string& _presetName, nlohmann::json& _outRaw, std::vector<std::string>& _outOrder) const {
    const std::string path = BuildPresetPath(_presetName);

    if (!std::filesystem::exists(path)) {
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        Log::Send(Log::Level::WARNING, std::format("Failed to open preset file: {}", path));
        return false;
    }

    json fileJson;
    file >> fileJson;
    file.close();

    _outRaw = json::object();
    for (auto& [name, data] : fileJson.items()) {
        if (name == "keyframes") continue;
        _outRaw[name] = data;
    }

    _outOrder.clear();
    if (fileJson.contains("keyframes")) {
        _outOrder = fileJson["keyframes"].get<std::vector<std::string>>();
    }

    return true;
}

std::pair<size_t, float> IPostEffect::ResolveKeyframeSegment(const float _t, const size_t _keyframeCount) {
    if (_keyframeCount <= 1) return { 0, 0.0f };

    const float segmentCount = static_cast<float>(_keyframeCount - 1);
    const float segmentProgress = _t * segmentCount;
    int currentSegment = static_cast<int>(segmentProgress);
    float segmentT = segmentProgress - static_cast<float>(currentSegment);

    if (currentSegment >= static_cast<int>(segmentCount)) {
        currentSegment = static_cast<int>(segmentCount) - 1;
        segmentT = 1.0f;
    }
    if (currentSegment < 0) {
        currentSegment = 0;
    }

    return { static_cast<size_t>(currentSegment), segmentT };
}

void IPostEffect::SaveKeyframeFile(const std::string& _presetName, const nlohmann::json& _keyframesObject, const std::vector<std::string>& _order) const {
    const std::string dir = "./Assets/Data/PostEffect/" + GetTypeName();
    std::filesystem::create_directories(dir);

    json fileJson = _keyframesObject;
    fileJson["keyframes"] = _order;

    std::ofstream file(BuildPresetPath(_presetName));
    if (!file.is_open()) {
        Log::Send(Log::Level::ERR, std::format("Failed to save preset '{}': {}", _presetName, BuildPresetPath(_presetName)));
        return;
    }

    file << fileJson.dump(4);
    file.close();
}
