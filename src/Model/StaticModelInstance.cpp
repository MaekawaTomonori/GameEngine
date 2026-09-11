#include "StaticModelInstance.hpp"

#include "Log.hpp"
#include "PerformanceProfiler.hpp"
#include "imgui.h"
#include "Pattern/Singleton.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/Texture/TextureManager.hpp"

void StaticModelInstance::Initialize(const std::string& _name) {
    InitializeCommon(_name);
}

void StaticModelInstance::Update() {
    PROFILE_SCOPE("Model - Mesh");
    mesh_->Update();
}

void StaticModelInstance::Draw() {
    if (!commandList_) {
        Log::Send(Log::Level::ERR, "Command list is null");
        return;
    }

    const bool isTransparent = mesh_->GetAlpha() < 1.0f;
    common_->RegisterStaticDraw(this, isTransparent, canvasName_);
}

void StaticModelInstance::ExecuteDraw() const {
    const auto tm = Singleton<TextureManager>::GetInstance();
    commandList_->SetGraphicsRootShaderResourceView(1, wr_->Get()->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootConstantBufferView(4, common_->GetCameraCBVAddress());
    commandList_->SetGraphicsRootDescriptorTable(8, tm->GetGPUHandle(environmentTexture_));
    mesh_->Draw();
}

void StaticModelInstance::Debug() {
    const std::string& label = name_.empty() ? data_->name : name_;
    ImGui::PushID(uuid_.c_str());
    if (ImGui::CollapsingHeader(label.c_str())) {
        DebugTransformSection();
        DebugMeshSection();
    }
    ImGui::PopID();
}
