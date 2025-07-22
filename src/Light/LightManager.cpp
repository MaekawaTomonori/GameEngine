#include "LightManager.hpp"

#include <algorithm>

#include "Log.hpp"
#include "Pattern/Singleton.hpp"
#include "imgui.h"
#include "src/Json/Json.hpp"
#include "vendor/MagicEnum/magic_enum.hpp"

void LightManager::Debug() {
    debug_->RegisterCommand("LM", [&](){
        if (ImGui::Begin("Light")){
            if (ImGui::BeginTabBar("Light")){
                if (ImGui::BeginTabItem("General")){
                    if(ImGui::CollapsingHeader("Files")){
                        if(ImGui::Button("Load / Reload")){Load();} ImGui::SameLine(); if(ImGui::Button("Save")){Save();}
                    }
                    if (ImGui::CollapsingHeader("Count")){
                        ImGui::Text("Directional: %d", static_cast<int>(lightCount_->dlCount));
                        ImGui::Text("Point: %d", static_cast<int>(lightCount_->plCount));
                        ImGui::Text("Spot: %d", static_cast<int>(lightCount_->slCount));
                    }
                    ImGui::PushID("Directional");
                    ImGui::SeparatorText("Directional");
                    if (ImGui::Button("Add")){Add(LightType::Directional);}
                    ImGui::PopID();
                    ImGui::PushID("Point");
                    ImGui::SeparatorText("Point");
                    if (ImGui::Button("Add")){Add(LightType::Point);}
                    ImGui::PopID();
                    ImGui::PushID("Spot");
                    ImGui::SeparatorText("Spot");
                    if (ImGui::Button("Add")){Add(LightType::Spot);}
                    ImGui::PopID();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Directional")){
                    for (const auto& dl : rawDirectionalLights_){dl->Update();}
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Point")){
                    for (const auto& pl : rawPointLights_){pl->Update();}
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Spot")){
                    for (const auto& sl : rawSpotLights_){sl->Update();}
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }
    });
}

void LightManager::CheckState() {
    Json* json = Singleton<Json>::GetInstance();
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
    Json* json = Singleton<Json>::GetInstance();
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

    Singleton<Json>::GetInstance()->Save(path);
}

void LightManager::Initialize(DirectXAdapter* _adapter, DebugUI* _debug) {
    adapter_ = _adapter;
    debug_ = _debug;
    commandList_ = adapter_->GetCommandList();

    // Light Counter
    countResource_ = std::make_unique<DX12Resource>();
    countResource_->Create(adapter_->CreateBufferResource(sizeof(LightCount)));
    countResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&lightCount_));

    lightCount_->dlCount = 0;
    lightCount_->plCount = 0;
    lightCount_->slCount = 0;

    //Directional
    directionalResource_ = std::make_unique<DX12Resource>();
    directionalResource_->Create(adapter_->CreateBufferResource(sizeof(DirectionalLight) * MAX_COUNT.dlCount));
    directionalResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mdDirectional_));

    //Point
    pointResource_ = std::make_unique<DX12Resource>();
    pointResource_->Create(adapter_->CreateBufferResource(sizeof(PointLight) * MAX_COUNT.plCount));
    pointResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mdPointLight_));

    //Spot
    spotResource_ = std::make_unique<DX12Resource>();
    spotResource_->Create(adapter_->CreateBufferResource(sizeof(SpotLight) * MAX_COUNT.slCount));
    spotResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mdSpotLight_));

    Load();

    Log::Send(Log::Level::INFO, "Light Enabled");
}

void LightManager::Update() {
    Debug();
    CheckState();

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
    commandList_->SetGraphicsRootShaderResourceView(3, directionalResource_->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootShaderResourceView(5, pointResource_->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootShaderResourceView(6, spotResource_->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootConstantBufferView(7, countResource_->GetGPUVirtualAddress());
}

void LightManager::Add(LightType type) {
    std::unique_ptr<RawDirectionalLight> directional;
    std::unique_ptr<RawPointLight> point;
    std::unique_ptr<RawSpotLight> spot;

    switch (type){
    case LightType::Directional:
        if (MAX_COUNT.dlCount <= ++lightCount_->dlCount){
            return;
        }
        directional = std::make_unique<RawDirectionalLight>();
        directional->DefaultSetting();
        rawDirectionalLights_.push_back(std::move(directional));
        break;
    case LightType::Point:
        if (MAX_COUNT.plCount <= ++lightCount_->plCount){
            return;
        }
        point = std::make_unique<RawPointLight>();
        point->DefaultSetting();
        rawPointLights_.push_back(std::move(point));
        break;
    case LightType::Spot:
        if (MAX_COUNT.slCount <= ++lightCount_->slCount){
            return;
        }
        spot = std::make_unique<RawSpotLight>();
        spot->DefaultSetting();
        rawSpotLights_.push_back(std::move(spot));
        break;
    }
}
