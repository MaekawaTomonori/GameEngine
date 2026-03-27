#include "LightManager.hpp"

#include <algorithm>

#include "Log.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Json/JsonParams.hpp"
#include "imgui.h"
#include "MagicEnum/magic_enum.hpp"

LightManager::~LightManager() {
    // Explicitly clear all resources
    directionalResource_.reset();
    pointResource_.reset();
    spotResource_.reset();
    countResource_.reset();

    rawDirectionalLights_.clear();
    rawPointLights_.clear();
    rawSpotLights_.clear();
}

void LightManager::Debug() {
#ifdef _DEBUG
    debug_->RegisterCommand("LightManager", [&](){
        ImGui::Begin("Light", &debug_->IsVisible("LightManager"));
        if (ImGui::BeginTabBar("LightTabs")){
            if (ImGui::BeginTabItem("General")){
                // ファイル操作
                if (ImGui::Button("Load/Reload")) { Load(); }
                ImGui::SameLine();
                if (ImGui::Button("Save")) { Save(); }

                ImGui::Separator();

                // Ref 制御
                const bool hasRef = ref_.has_value();
                if (!hasRef) ImGui::BeginDisabled();
                ImGui::Checkbox("Follow Ref", &refEnabled_);
                if (!hasRef) {
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    ImGui::TextDisabled("(no ref set)");
                }

                if (ImGui::Button("Clear Ref")) {
                    ClearRef();
                }

                ImGui::Separator();

                // ライト数と追加ボタン
                const bool dlFull = lightCount_->dlCount >= MAX_COUNT.dlCount;
                const bool plFull = lightCount_->plCount >= MAX_COUNT.plCount;
                const bool slFull = lightCount_->slCount >= MAX_COUNT.slCount;

                ImGui::Text("Directional : %u / %u", lightCount_->dlCount, MAX_COUNT.dlCount);
                ImGui::SameLine();
                if (dlFull) ImGui::BeginDisabled();
                if (ImGui::SmallButton("+ DL")) { Add(LightType::Directional); }
                if (dlFull) ImGui::EndDisabled();

                ImGui::Text("Point       : %u / %u", lightCount_->plCount, MAX_COUNT.plCount);
                ImGui::SameLine();
                if (plFull) ImGui::BeginDisabled();
                if (ImGui::SmallButton("+ PL")) { Add(LightType::Point); }
                if (plFull) ImGui::EndDisabled();

                ImGui::Text("Spot        : %u / %u", lightCount_->slCount, MAX_COUNT.slCount);
                ImGui::SameLine();
                if (slFull) ImGui::BeginDisabled();
                if (ImGui::SmallButton("+ SL")) { Add(LightType::Spot); }
                if (slFull) ImGui::EndDisabled();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Directional")){
                for (int i = 0; i < static_cast<int>(rawDirectionalLights_.size()); ++i) {
                    rawDirectionalLights_[i]->ImGuiSetting(i);
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Point")){
                for (int i = 0; i < static_cast<int>(rawPointLights_.size()); ++i) {
                    rawPointLights_[i]->ImGuiSetting(i);
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Spot")){
                for (int i = 0; i < static_cast<int>(rawSpotLights_.size()); ++i) {
                    rawSpotLights_[i]->ImGuiSetting(i);
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    });
#endif
}

void LightManager::CheckState() {
    JsonParams* json = Singleton<JsonParams>::GetInstance();
    std::erase_if(rawDirectionalLights_, [&](const std::unique_ptr<RawDirectionalLight>& _dl){
        if (!_dl->IsEnable()){
            lightCount_->dlCount--;
            json->RemoveGroup(path, _dl->GetUUID());
            return true;
        }
        return false;
    });
    std::erase_if(rawPointLights_, [&](const std::unique_ptr<RawPointLight>& _pl){
        if (!_pl->IsEnable()){
            lightCount_->plCount--;
            json->RemoveGroup(path, _pl->GetUUID());
            return true;
        }
        return false;
    });
    std::erase_if(rawSpotLights_, [&](const std::unique_ptr<RawSpotLight>& _sl){
        if (!_sl->IsEnable()){
            lightCount_->slCount--;
            json->RemoveGroup(path, _sl->GetUUID());
            return true;
        }
        return false;
    });
}

void LightManager::Load() {
    JsonParams* json = Singleton<JsonParams>::GetInstance();
    json->Load(path);

    rawDirectionalLights_.clear();
    rawPointLights_.clear();
    rawSpotLights_.clear();
    *lightCount_ = {0,0,0};
    auto data = json->GetGroups(path);
    for (auto& itr : data) {
        auto group = itr.second;
        switch (magic_enum::enum_value<LightType>(std::get<int32_t>(group.at("type")))){
            case LightType::Directional:
                Add(LightType::Directional);
                rawDirectionalLights_.back()->Set(itr.first, {
                    std::get<Vector4>(group.at("color")),
                    std::get<Vector3>(group.at("direction")),
                    std::get<float>(group.at("intensity"))
                });
                break;
            case LightType::Point:
                Add(LightType::Point);
                rawPointLights_.back()->Set(itr.first, {
                    std::get<Vector4>(group.at("color")),
                    std::get<Vector3>(group.at("position")),
                    std::get<float>(group.at("intensity")),
                    std::get<float>(group.at("radius")),
                    std::get<float>(group.at("decay")),
                    {0,0}
                });
                break;
            case LightType::Spot:
                Add(LightType::Spot);
                rawSpotLights_.back()->Set(itr.first, {
                    std::get<Vector4>(group.at("color")),
                    std::get<Vector3>(group.at("position")),
                    std::get<float>(group.at("intensity")),
                    std::get<Vector3>(group.at("direction")).Normalize(),
                    std::get<float>(group.at("distance")),
                    std::get<float>(group.at("decay")),
                    std::get<float>(group.at("cosAngle")),
                    std::get<float>(group.at("falloffStart")),
                    0
                });
                break;
        }
    }
}

void LightManager::Save() const {
    for (const auto& dl : rawDirectionalLights_){
        dl->Save(path);
    }

    for (const auto& pl : rawPointLights_){
        pl->Save(path);
    }

    for (const auto& sl : rawSpotLights_){
        sl->Save(path);
    }

    Singleton<JsonParams>::GetInstance()->Save(path);
}

void LightManager::Initialize(DirectXAdapter* _adapter, DebugUI* _debug) {
    adapter_ = _adapter;
    debug_ = _debug;
    commandList_ = adapter_->GetCommandList();

    // Light Counter
    countResource_ = std::make_unique<DX12Resource>();
    countResource_= adapter_->CreateBufferResource(sizeof(LightCount));
    countResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&lightCount_));

    lightCount_->dlCount = 0;
    lightCount_->plCount = 0;
    lightCount_->slCount = 0;

    //Directional
    directionalResource_ = std::make_unique<DX12Resource>();
    directionalResource_ = adapter_->CreateBufferResource(sizeof(DirectionalLight) * MAX_COUNT.dlCount);
    directionalResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mdDirectional_));

    //Point
    pointResource_ = std::make_unique<DX12Resource>();
    pointResource_ = adapter_->CreateBufferResource(sizeof(PointLight) * MAX_COUNT.plCount);
    pointResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mdPointLight_));

    //Spot
    spotResource_ = std::make_unique<DX12Resource>();
    spotResource_ = adapter_->CreateBufferResource(sizeof(SpotLight) * MAX_COUNT.slCount);
    spotResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mdSpotLight_));

    Load();

    debug_->RegisterMenuButton("LightManager");

    Log::Send(Log::Level::INFO, "Light Enabled");
}

void LightManager::Update() {
    CheckState();
    UpdateLights();

    // Apply to GPU (raw data -mapping-> gpu data)
    uint32_t index = 0;
    for (index = 0; index < lightCount_->dlCount; ++index){
        mdDirectional_[index] = rawDirectionalLights_[index]->GetLight();
    }
    for (index = 0; index < lightCount_->plCount; ++index){
        mdPointLight_[index] = rawPointLights_[index]->GetLight();
    }
    for (index = 0; index < lightCount_->slCount; ++index){
        mdSpotLight_[index] = rawSpotLights_[index]->GetLight();
    }
}

void LightManager::Draw() const {
    commandList_->SetGraphicsRootShaderResourceView(3, directionalResource_->Get()->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootShaderResourceView(5, pointResource_->Get()->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootShaderResourceView(6, spotResource_->Get()->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootConstantBufferView(7, countResource_->Get()->GetGPUVirtualAddress());
}

void LightManager::Add(LightType _type) {
    std::unique_ptr<RawDirectionalLight> directional;
    std::unique_ptr<RawPointLight> point;
    std::unique_ptr<RawSpotLight> spot;

    switch (_type){
    case LightType::Directional:
        if (MAX_COUNT.dlCount <= ++lightCount_->dlCount){
            return;
        }
        directional = std::make_unique<RawDirectionalLight>();
        directional->DefaultSetting();
        if (ref_.has_value()){
            directional->SetReference(ref_.value());
        }
        rawDirectionalLights_.push_back(std::move(directional));
        break;
    case LightType::Point:
        if (MAX_COUNT.plCount <= ++lightCount_->plCount){
            return;
        }
        point = std::make_unique<RawPointLight>();
        point->DefaultSetting();
        if (ref_.has_value()){
            point->SetReference(ref_.value());
        }
        rawPointLights_.push_back(std::move(point));
        break;
    case LightType::Spot:
        if (MAX_COUNT.slCount <= ++lightCount_->slCount){
            return;
        }
        spot = std::make_unique<RawSpotLight>();
        spot->DefaultSetting();
        if (ref_.has_value()){
            spot->SetReference(ref_.value());
        }
        rawSpotLights_.push_back(std::move(spot));
        break;
    }
}

void LightManager::SetPosition(const Vector3& _pos) {
    ref_ = _pos;

    for (const auto& dl : rawDirectionalLights_) {
        dl->SetReference(_pos);
    }

    for (const auto& pl : rawPointLights_) {
        pl->SetReference(_pos);
    }

    for (const auto& sl : rawSpotLights_) {
        sl->SetReference(_pos);
    }
}

void LightManager::ClearRef() {
    ref_.reset();

    for (const auto& dl : rawDirectionalLights_) {
        dl->ClearRef();
    }

    for (const auto& pl : rawPointLights_) {
        pl->ClearRef();
    }

    for (const auto& sl : rawSpotLights_) {
        sl->ClearRef();
    }
}

void LightManager::UpdateLights() {
    if (!refEnabled_) return;

    for (const auto& dl : rawDirectionalLights_) {
        dl->Update();
    }

    for (const auto& pl : rawPointLights_) {
        pl->Update();
    }

    for (const auto& sl : rawSpotLights_) {
        sl->Update();
    }
}
