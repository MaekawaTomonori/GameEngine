#include "Model.hpp"

#include <filesystem>

#include "Log.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "Pattern/Singleton.hpp"
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
    Log::Send(Log::Level::INFO, "Model::Initialize: " + _name);

    Load(_name);

    Log::Send(Log::Level::INFO, "Model data loaded: " + _name);

    data_ = common_->GetResourceRepository()->GetModelRepository()->Get(_name);

    Log::Send(Log::Level::INFO, "Creating Mesh: " + _name);

    mesh_ = std::make_unique<Mesh>();
    mesh_->Initialize(adapter_, _name, common_->GetResourceRepository()->GetMeshRepository()->Get(data_->mesh));

    Log::Send(Log::Level::INFO, "Mesh created: " + _name);

    wr_.Attach(adapter_->CreateBufferResource(sizeof(Transformation)));
    wr_->Map(0, nullptr, reinterpret_cast<void**>(&wd_));
    wd_->wvp = MathUtils::Matrix::MakeIdentity();
    wd_->world = MathUtils::Matrix::MakeIdentity();
    wd_->inverse = MathUtils::Matrix::MakeIdentity();

    cr_.Attach(adapter_->CreateBufferResource(sizeof(CameraForGpu)));
    cr_->Map(0, nullptr, reinterpret_cast<void**>(&cd_));

    Log::Send(Log::Level::INFO, "Creating SkinCluster for: " + _name);
    CreateSkinCluster();
    Log::Send(Log::Level::INFO, "SkinCluster created for: " + _name);


    transform_ = {
        {1,1,1},
        Vector3{0,0,0},
        {0,0,0},
    };

    line_.Initialize();
}

void Model::Update() {
    camera_ = Singleton<CameraManager>::GetInstance()->GetActive();

    Debug();

    // Update Animation
    UpdateAnimation();

    // Update Skeleton
    UpdateSkeleton();

    // Update SkinCluster
    UpdateSkinCluster();

    UpdateMapData();

    // Joint to Line
    CreateLine();
}

void Model::Draw() const {
    if (!commandList_){
        Log::Send(Log::Level::ERR, "Command list is null");
        return;
    }

    common_->Draw();

    commandList_->SetGraphicsRootConstantBufferView(1, wr_->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootConstantBufferView(4, cr_->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootDescriptorTable(8, skinCluster_.paletteHandle.second);

    mesh_->SetVBV(skinCluster_.influenceBufferView);
    mesh_->Draw();

    DrawLine();
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
                ImGui::SeparatorText("Model Info");
                if (ImGui::TreeNode("Transform")){
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
                    ImGui::TreePop();
                }

                ImGui::SeparatorText("Skeleton");
                std::function<void(int32_t)> Recursive = [&](int32_t index){
                    Joint& joint = data_->skeleton.joints[index];
                    if (ImGui::TreeNode(joint.name.c_str())) {
                        ImGui::Text("Joint: %s", joint.name.c_str());
                        ImGui::Text("Index: %d", joint.index);
                        ImGui::Text("Parent: %s", joint.parent.has_value() ? data_->skeleton.joints[joint.parent.value()].name.c_str() : "None");
                        ImGui::Text("Children: %zu", joint.children.size());
                        ImGui::Text("Transform: ");
                        ImGui::Text("  Scale: (%.2f, %.2f, %.2f)", joint.transform.scale.x, joint.transform.scale.y, joint.transform.scale.z);
                        if (std::holds_alternative<Quaternion>(joint.transform.rotate)){
                            Quaternion rotate = std::get<Quaternion>(joint.transform.rotate);
                            ImGui::Text("  Rotate: (%.2f, %.2f, %.2f, %2f)", rotate.x, rotate.y, rotate.z, rotate.w);
                        } else {
                            Vector3 rotate = std::get<Vector3>(joint.transform.rotate);
                            ImGui::Text("  Rotate: (%.2f, %.2f, %.2f)", rotate.x, rotate.y, rotate.z);
                        }
                        ImGui::Text("  Translate: (%.2f, %.2f, %.2f)", joint.transform.translate.x, joint.transform.translate.y, joint.transform.translate.z);
                        

                        for (int32_t childIndex : joint.children){
                            Recursive(childIndex);
                        }
                        ImGui::TreePop();
                    }
                };
                Recursive(data_->skeleton.root);

                ImGui::SeparatorText("Animation");
                if (ImGui::TreeNode("Details")){
                    ImGui::Checkbox("Enable", &animationEnable_);
                    ImGui::SameLine();
                    ImGui::Checkbox("TimerLock", &animationTimerLock_);
                    if (animationTimerLock_)ImGui::Text("Timer : %f", animationTime_);
                    else ImGui::DragFloat("Anime Timer", &animationTime_);
                    ImGui::TreePop();
                }

                if (ImGui::CollapsingHeader("Mesh")){
                    mesh_->Debug();
                }
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
    size_t jointCount = data_->skeleton.joints.size();
    skinCluster_.paletteResource.Attach(adapter_->CreateBufferResource(sizeof(WellForGpu) * jointCount));
    WellForGpu* mappedPalette = nullptr;
    skinCluster_.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
    skinCluster_.mappedPalette = {mappedPalette, jointCount};
    skinCluster_.srvIndex = common_->GetSRVManager()->Allocate();
    common_->GetSRVManager()->CreateSRVforStructuredBuffer(skinCluster_.srvIndex, skinCluster_.paletteResource.Get(), static_cast<UINT>(data_->skeleton.joints.size()), sizeof(WellForGpu));
    skinCluster_.paletteHandle = {
        common_->GetSRVManager()->GetCPUHandle(skinCluster_.srvIndex),
        common_->GetSRVManager()->GetGPUHandle(skinCluster_.srvIndex)
    };

    size_t verticesSize = common_->GetResourceRepository()->GetMeshRepository()->Get(data_->mesh).vertices.size();
    skinCluster_.influenceResource.Attach(adapter_->CreateBufferResource(sizeof(VertexInfluence) * verticesSize));
    VertexInfluence* mappedInfluence = nullptr;
    skinCluster_.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
    memset(mappedInfluence, 0, sizeof(VertexInfluence) * verticesSize);
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

        size_t jointIndex = itr->second;
        if (jointIndex >= data_->skeleton.joints.size()){
            Log::Send(Log::Level::ERR, "Joint index out of bounds in skin cluster creation for joint: " + jointWeight.first);
            Utils::Alert("Joint index out of bounds in skin cluster creation");
            continue;
        }
        if (jointIndex >= skinCluster_.mappedPalette.size()) {
            Log::Send(Log::Level::ERR, "Joint index out of bounds in skin cluster creation for joint: " + jointWeight.first);
            Utils::Alert("Joint index out of bounds in skin cluster creation");
            continue;
        }

        skinCluster_.bindPoseMatrices[jointIndex] = jointWeight.second.inverseBindPose;

        for (const auto& vertexWeight : jointWeight.second.weights) {
            if (verticesSize <= vertexWeight.index) {
                Log::Send(Log::Level::ERR, "Vertex index out of bounds in skin cluster creation for joint: " + jointWeight.first);
                Utils::Alert("Vertex index out of bounds in skin cluster creation");
                continue;
            }

            auto& currentInfluence = skinCluster_.mappedInfluence[vertexWeight.index];
            for (uint32_t index = 0; index < MAX_INFLUENCE; ++index) {
                if (currentInfluence.weights[index] == 0.0f){
                    currentInfluence.weights[index] = vertexWeight.weight;
                    currentInfluence.jointIndices[index] = static_cast<int32_t>(jointIndex);
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

void Model::UpdateSkeleton() const {
    std::function<void(int32_t)> RecursiveUpdate = [&](int32_t index){
        Joint& joint = data_->skeleton.joints[index];
        joint.local = MathUtils::Matrix::MakeAffineMatrix(joint.transform);
        if (joint.parent){
            joint.space = joint.local * data_->skeleton.joints[*joint.parent].space;
        } else{
            joint.space = joint.local;
        }

        for (int32_t child : joint.children) {
            RecursiveUpdate(child);
        }
    };

    RecursiveUpdate(data_->skeleton.root);
}

void Model::UpdateAnimation() {
    // Update AnimationTimer
    if (animationEnable_){
        animationTime_ += 1.f;
        animationTime_ = fmod(animationTime_, data_->animation.duration);
    }

    // ApplyAnimation
    ApplyAnimation();
}

void Model::ApplyAnimation() const {
    for (Joint& joint : data_->skeleton.joints) {
        if (data_->animation.nodeAnimations.contains(joint.name)) {
            auto& rna = data_->animation.nodeAnimations[joint.name];

            joint.transform.scale = AnimationCurveFunction::Calculate(rna.scale, animationTime_);
            joint.transform.rotate = AnimationCurveFunction::Calculate(rna.rotation, animationTime_);
            joint.transform.translate = AnimationCurveFunction::Calculate(rna.translate, animationTime_);
        }
    }
}

void Model::CreateLine() {
    line_.Clear();
        
    for (auto& joint : data_->skeleton.joints){
        if (joint.parent.has_value()){
            line_.AddLine(joint.space.GetTranslate(), data_->skeleton.joints[*joint.parent].space.GetTranslate());
        }
    }
    line_.Update();
}

void Model::DrawLine() const {
    line_.Draw();
}
