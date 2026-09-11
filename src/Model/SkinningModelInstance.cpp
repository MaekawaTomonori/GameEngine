#include "SkinningModelInstance.hpp"

#include "Log.hpp"
#include "PerformanceProfiler.hpp"
#include "imgui.h"
#include "Pattern/Singleton.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/Texture/TextureManager.hpp"

void SkinningModelInstance::Initialize(const std::string& _name) {
    InitializeCommon(_name);

    skinning_ = std::make_unique<SkinningState>();
    skinning_->Initialize(adapter_, common_, data_, *mesh_);
}

void SkinningModelInstance::Update() {
    { PROFILE_SCOPE("Model - Skinning"); skinning_->Update(); }
    { PROFILE_SCOPE("Model - Mesh"); mesh_->Update(); }
}

void SkinningModelInstance::Draw() {
    if (!commandList_) {
        Log::Send(Log::Level::ERR, "Command list is null");
        return;
    }

    const bool isTransparent = mesh_->GetAlpha() < 1.0f;
    common_->RegisterSkinningDraw(this, isTransparent, canvasName_);

#ifdef _DEBUG
    skinning_->DrawLine();
#endif
}

void SkinningModelInstance::ExecuteDraw() const {
    const auto tm = Singleton<TextureManager>::GetInstance();
    commandList_->SetGraphicsRootConstantBufferView(1, wr_->Get()->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootConstantBufferView(4, common_->GetCameraCBVAddress());
    commandList_->SetGraphicsRootDescriptorTable(8, tm->GetGPUHandle(environmentTexture_));
    commandList_->SetGraphicsRootDescriptorTable(11, skinning_->GetPaletteHandle());
    mesh_->Draw();
}

void SkinningModelInstance::Debug() {
    const std::string& label = name_.empty() ? data_->name : name_;
    ImGui::PushID(uuid_.c_str());
    if (ImGui::CollapsingHeader(label.c_str())) {
        DebugTransformSection();
        skinning_->Debug(uuid_);
        DebugMeshSection();
    }
    ImGui::PopID();
}
