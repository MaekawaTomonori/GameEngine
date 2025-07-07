#include "Model.hpp"

#include "Log.hpp"
#include "Pattern/Singleton.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "Math/MathUtils.hpp"
#include "src/Camera/Manager/CameraManager.hpp"
#include "src/Mesh/Loader/MeshManager.hpp"

Model::Model() :
    common_(Singleton<ModelCommon>::GetInstance()),
    adapter_(common_->GetAdapter()),
    commandList_(adapter_->GetCommandList()),
    uuid_(Utils::GenerateUniqueId()), transform_() {
}

void Model::Initialize(const std::string &_name) {
    if (!adapter_){
        Log::Send(Log::Level::ERR, "Adapter is null");
        return;
    }

    SetMesh(_name);

    wr_.Attach(adapter_->CreateBufferResource(sizeof(Transformation)));
    wr_->Map(0, nullptr, reinterpret_cast<void**>(&wd_));
    wd_->wvp = MathUtils::Matrix::MakeIdentity();
    wd_->world = MathUtils::Matrix::MakeIdentity();
    wd_->inverse = MathUtils::Matrix::MakeIdentity();

    cr_.Attach(adapter_->CreateBufferResource(sizeof(CameraForGpu)));
    cr_->Map(0, nullptr, reinterpret_cast<void**>(&cd_));

    transform_ = {
        {1,1,1},
        {0,0,0},
        {0,0,0},
    };
}

void Model::Update() {
    Debug();

    camera_ = Singleton<CameraManager>::GetInstance()->GetActive();
    wd_->world = MathUtils::Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    wd_->wvp = wd_->world * camera_->GetViewProjection();
    wd_->inverse = wd_->world.Inverse().Transpose();

    *cd_ = camera_->GetCameraForGpu();
}

void Model::Draw() const {
    if (!commandList_){
        Log::Send(Log::Level::ERR, "Command list is null");
        return;
    }
    if (!mesh_){
        Log::Send(Log::Level::ERR, "Mesh not set for model: " + meshName_);
        return;
    }

    common_->Draw();

    commandList_->SetGraphicsRootConstantBufferView(1, wr_->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootConstantBufferView(4, cr_->GetGPUVirtualAddress());

    mesh_->Draw();
}

void Model::Debug() {
    common_->RegisterCommand(
        uuid_, [&]() {
            ImGui::Begin("Model");
            if (ImGui::CollapsingHeader(uuid_.c_str())) {
                ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);
                ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.1f);
                ImGui::DragFloat3("Position", &transform_.translate.x, 0.1f);
                if (ImGui::Button("Reset Transform")){
                    transform_ = {
                        {1, 1, 1},
                        {0, 0, 0},
                        {0, 0, 0},
                    };
                }

                ImGui::SeparatorText("Mesh");
                ImGui::Text("Name: %s", meshName_.c_str());
                mesh_->Debug();
            }
            ImGui::End();
        }
    );
}

void Model::SetMesh(const std::string &_name) {
    meshName_ = _name;

    mesh_ = Singleton<MeshManager>::GetInstance()->Load(_name);
}
