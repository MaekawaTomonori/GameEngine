#include "Model.hpp"

#include <algorithm>
#include <filesystem>

#include "Log.hpp"
#include "Pattern/Singleton.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "Loader/GltfLoader.hpp"
#include "Loader/IModelLoader.hpp"
#include "Loader/ObjLoader.hpp"
#include "Math/MathUtils.hpp"
#include "src/Camera/Manager/CameraManager.hpp"

Model::Model() :
    common_(Singleton<ModelCommon>::GetInstance()),
    adapter_(common_->GetAdapter()),
    commandList_(adapter_->GetCommandList()),
    uuid_(Utils::GenerateUniqueId()), transform_() {
}

void Model::Initialize(const std::string& _name) {
    if (!adapter_){
        Log::Send(Log::Level::ERR, "Adapter is null");
        return;
    }

    Load(_name);


    data_ = common_->GetResourceRepository()->GetModelRepository()->Get(_name);

    mesh_ = std::make_unique<Mesh>();
    mesh_->Initialize(adapter_, _name, common_->GetResourceRepository()->GetMeshRepository()->Get(data_->mesh));

    wr_.Attach(adapter_->CreateBufferResource(sizeof(Transformation)));
    wr_->Map(0, nullptr, reinterpret_cast<void**>(&wd_));
    wd_->wvp = MathUtils::Matrix::MakeIdentity();
    wd_->world = MathUtils::Matrix::MakeIdentity();
    wd_->inverse = MathUtils::Matrix::MakeIdentity();

    cr_.Attach(adapter_->CreateBufferResource(sizeof(CameraForGpu)));
    cr_->Map(0, nullptr, reinterpret_cast<void**>(&cd_));

    CreateSkinCluster();

    mesh_->SetVBV(skinCluster_.influenceBufferView);

    transform_ = {
        {1,1,1},
        Vector3{0,0,0},
        {0,0,0},
    };
}

void Model::Update() {
    Debug();

    // Update AnimationTimer

    // ApplyAnimation
    ApplyAnimation();

    // Update Skeleton
    UpdateSkeleton();

    //Update SkinCluster
    UpdateSkinCluster();

    camera_ = Singleton<CameraManager>::GetInstance()->GetActive();
    UpdateMapData();
}

void Model::Draw() const {
    if (!commandList_){
        Log::Send(Log::Level::ERR, "Command list is null");
        return;
    }
    
    common_->Draw();

    commandList_->SetGraphicsRootConstantBufferView(1, wr_->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootConstantBufferView(4, cr_->GetGPUVirtualAddress());

    mesh_->Draw();
}

void Model::Load(const std::string& _name) const {
    std::unique_ptr<IModelLoader> loader;
    if (std::filesystem::exists("Assets/Resources/" + _name + "/" + _name + ".obj")) {
        loader = std::make_unique<ObjLoader>();
    } else if (std::filesystem::exists("Assets/Resources/" + _name + "/" + _name + ".gltf")){
        loader = std::make_unique<GltfLoader>();
    } else{
        Log::Send(Log::Level::ERR, "Model::Load: Model not found: " + _name);
        Utils::Alert("Model not found: " + _name);
        return;
    }
    loader->LoadModel(_name, common_->GetResourceRepository());
}

void Model::Debug() {
    common_->RegisterCommand(
        uuid_, [&]() {
            ImGui::Begin("Model");
            if (ImGui::CollapsingHeader(uuid_.c_str())) {
                ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);
                ImGui::DragFloat3("Rotate", &std::get<Vector3>(transform_.rotate).x, 0.1f);
                ImGui::DragFloat3("Position", &transform_.translate.x, 0.1f);
                if (ImGui::Button("Reset Transform")){
                    transform_ = Transform{
                        {1, 1, 1},
                        Vector3{0, 0, 0},
                        {0, 0, 0},
                    };
                }

                ImGui::SeparatorText("Mesh");
            }
            ImGui::End();
        }
    );
}

void Model::UpdateMapData() const {
    wd_->world = MathUtils::Matrix::MakeAffineMatrix(transform_.scale, std::get<Vector3>(transform_.rotate), transform_.translate);
    wd_->wvp = wd_->world * camera_->GetViewProjection();
    wd_->inverse = wd_->world.Inverse().Transpose();

    *cd_ = camera_->GetCameraForGpu();
}

void Model::CreateSkinCluster() {
    Microsoft::WRL::ComPtr<ID3D12Device> device = adapter_->GetDevice();
    if (!device){
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return;
    }

    skinCluster_.paletteResource.Attach(adapter_->CreateBufferResource(sizeof(WellForGpu) * data_->skeleton.joints.size()));
    WellForGpu* mappedPalette = nullptr;
    skinCluster_.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
    skinCluster_.mappedPalette = {mappedPalette, data_->skeleton.joints.size()};
    skinCluster_.srvIndex = common_->GetSRVManager()->Allocate();
    skinCluster_.paletteHandle = {
        common_->GetSRVManager()->GetCPUHandle(skinCluster_.srvIndex),
        common_->GetSRVManager()->GetGPUHandle(skinCluster_.srvIndex)
    };
    common_->GetSRVManager()->CreateSRVforStructuredBuffer(skinCluster_.srvIndex, skinCluster_.paletteResource.Get(), static_cast<UINT>(data_->skeleton.joints.size()), sizeof(WellForGpu));

    size_t verticesSize = common_->GetResourceRepository()->GetMeshRepository()->Get(data_->mesh).vertices.size();
    skinCluster_.influenceResource.Attach(adapter_->CreateBufferResource(sizeof(VertexInfluence) * verticesSize));
    VertexInfluence* mappedInfluence = nullptr;
    skinCluster_.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
    skinCluster_.mappedInfluence = { mappedInfluence, verticesSize };

    skinCluster_.influenceBufferView.BufferLocation = skinCluster_.influenceResource->GetGPUVirtualAddress();
    skinCluster_.influenceBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexInfluence) * verticesSize);
    skinCluster_.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

    skinCluster_.bindPoseMatrices.resize(data_->skeleton.joints.size());
    for (auto& bindPoseMatrix : skinCluster_.bindPoseMatrices) {
        bindPoseMatrix = MathUtils::Matrix::MakeIdentity();
    }

    for (const auto& jointWeight : data_->skinCluster) {
        auto itr = data_->skeleton.map.find(jointWeight.first);
        if (itr == data_->skeleton.map.end()){
            Log::Send(Log::Level::ERR, "Joint not found in skeleton: " + jointWeight.first);
            continue;
        }

        skinCluster_.bindPoseMatrices[itr->second] = jointWeight.second.inverseBindPose;
        for (const auto& vertexWeight : jointWeight.second.weights) {
            auto& currentInfluence = skinCluster_.mappedInfluence[vertexWeight.index];
            for (uint32_t index = 0; index < MAX_INFLUENCE; ++index) {
                if (currentInfluence.weights[index] == 0.0f){
                    currentInfluence.weights[index] = vertexWeight.weight;
                    currentInfluence.jointIndices[index] = itr->second;
                    break;
                }
            }
        }
    }
}

void Model::UpdateSkinCluster() {
    for (size_t jointIndex = 0; jointIndex < data_->skeleton.joints.size(); ++jointIndex) {
        if (skinCluster_.bindPoseMatrices.size() <= jointIndex) {
            Utils::Alert("Joint index out of bounds in skin cluster update");
            break;
        }

        skinCluster_.mappedPalette[jointIndex].space = skinCluster_.bindPoseMatrices[jointIndex] * data_->skeleton.joints[jointIndex].space;
        skinCluster_.mappedPalette[jointIndex].inverseTranspose = skinCluster_.mappedPalette[jointIndex].space.Inverse().Transpose();
    }
}

void Model::ApplyAnimation() {
    for (Joint& joint : data_->skeleton.joints) {
        if (data_->animation.nodeAnimations.contains(joint.name)) {
            const NodeAnimation& rna = data_->animation.nodeAnimations[joint.name];
            (void)rna;
            //joint.transform.scale = rna.scale.keyframes;
        }
    }
}

void Model::UpdateSkeleton() {
    for (Joint& joint : data_->skeleton.joints){
        joint.local = MathUtils::Matrix::MakeAffineMatrix(joint.transform);
        if (joint.parent){
            joint.space = joint.local * data_->skeleton.joints[*joint.parent].space;
        } else{
            joint.space = joint.local;
        }
    }
}