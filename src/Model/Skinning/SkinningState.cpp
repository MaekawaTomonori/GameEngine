#include "SkinningState.hpp"

#include <cstring>
#include <functional>
#include <variant>

#include "Log.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "Math/MathUtils.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Vector3.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/Mesh/Mesh.hpp"
#include "src/Model/Common/ModelCommon.hpp"

void SkinningState::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<ModelCommon>& _common, const GESTD::ReferencePtr<ModelData>& _data, Mesh& _mesh) {
    adapter_ = _adapter;
    common_ = _common;
    data_ = _data;
    pose_ = data_->skeleton.value();

    CreateSkinCluster(_mesh);

    line_.Initialize();
}

void SkinningState::Update() {
    UpdateAnimation();
    UpdateSkeleton();
    UpdateSkinCluster();

#ifdef _DEBUG
    CreateLine();
#endif
}

void SkinningState::Debug(const std::string& _uuidPrefix) {
    ImGui::PushID(("skeleton_" + _uuidPrefix).c_str());
    ImGui::SeparatorText("Skeleton");
    std::function<void(int32_t)> Recursive = [&](int32_t _index) {
        Joint& joint = pose_.joints[_index];
        if (ImGui::TreeNode(joint.name.c_str())) {
            ImGui::Text("Joint: %s", joint.name.c_str());
            ImGui::Text("Index: %d", joint.index);
            ImGui::Text("Parent: %s", joint.parent.has_value() ? pose_.joints[joint.parent.value()].name.c_str() : "None");
            ImGui::Text("Children: %zu", joint.children.size());
            ImGui::Spacing();
            ImGui::Text("Transform: ");
            ImGui::Text("  Scale: (%.2f, %.2f, %.2f)", joint.transform.scale.x, joint.transform.scale.y, joint.transform.scale.z);
            if (std::holds_alternative<Quaternion>(joint.transform.rotate)) {
                Quaternion rotate = std::get<Quaternion>(joint.transform.rotate);
                ImGui::Text("  Rotate: (%.2f, %.2f, %.2f, %2f)", rotate.x, rotate.y, rotate.z, rotate.w);
            }
            else {
                Vector3 rotate = std::get<Vector3>(joint.transform.rotate);
                ImGui::Text("  Rotate: (%.2f, %.2f, %.2f)", rotate.x, rotate.y, rotate.z);
            }
            ImGui::Text("  Translate: (%.2f, %.2f, %.2f)", joint.transform.translate.x, joint.transform.translate.y, joint.transform.translate.z);

            for (int32_t childIndex : joint.children) {
                ImGui::Spacing();
                Recursive(childIndex);
            }
            ImGui::TreePop();
        }
        };
    Recursive(pose_.root);
    ImGui::PopID();

    if (data_->animation.has_value()) {
        ImGui::SeparatorText("Animation");
        if (ImGui::TreeNode("Details")) {
            ImGui::Checkbox("Enable", &animationEnable_);
            ImGui::SameLine();
            ImGui::Checkbox("TimerLock", &animationTimerLock_);
            if (animationTimerLock_) ImGui::Text("Timer : %f", animationTime_);
            else ImGui::DragFloat("Anime Timer", &animationTime_);
            ImGui::TreePop();
        }
    }
}

void SkinningState::DrawLine() const {
    line_.Draw();
}

void SkinningState::CreateSkinCluster(Mesh& _mesh) {
    Microsoft::WRL::ComPtr<ID3D12Device> device = adapter_->GetDevice();
    if (!device) {
        Log::Send(Log::Level::ERR, "DirectXAdapter is not initialized");
        return;
    }

    SRVManager* srv = common_->GetSRVManager();
    if (!srv) {
        Log::Send(Log::Level::ERR, "SRVManager is not initialized");
        Utils::Alert("SRVManager is not initialized");
        return;
    }

    size_t jointSize = pose_.joints.size();
    size_t verticesSize = _mesh.GetData().vertices.size();

    //Palette
    WellForGpu* mappedPalette = nullptr;
    skinCluster_.paletteResource = (adapter_->CreateBufferResource(sizeof(WellForGpu) * jointSize));
    skinCluster_.paletteResource->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
    skinCluster_.mappedPalette = { mappedPalette, jointSize };
    skinCluster_.srvIndex = srv->Allocate();
    skinCluster_.paletteHandle = { srv->GetCPUHandle(skinCluster_.srvIndex), srv->GetGPUHandle(skinCluster_.srvIndex) };
    srv->CreateSRVForStructuredBuffer(skinCluster_.srvIndex, skinCluster_.paletteResource->Get(), static_cast<UINT>(jointSize), sizeof(WellForGpu));

    //Influenc
    VertexInfluence* mappedInfluence = nullptr;
    skinCluster_.influenceResource = (adapter_->CreateBufferResource(sizeof(VertexInfluence) * verticesSize));
    skinCluster_.influenceResource->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
    memset(mappedInfluence, 0, sizeof(VertexInfluence) * verticesSize);
    skinCluster_.mappedInfluence = { mappedInfluence, verticesSize };

    skinCluster_.influenceBufferView.BufferLocation = skinCluster_.influenceResource->Get()->GetGPUVirtualAddress();
    skinCluster_.influenceBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexInfluence) * verticesSize);
    skinCluster_.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

    SetBindPose();

    _mesh.SetVBV(skinCluster_.influenceBufferView);
}

void SkinningState::SetBindPose() {
    size_t jointSize = pose_.joints.size();
    skinCluster_.inverseBindPoses.resize(jointSize);
    for (auto& inverseBindPose : skinCluster_.inverseBindPoses) {
        inverseBindPose = MathUtils::Matrix::MakeIdentity();
    }

    for (const auto& jointWeight : data_->skinCluster) {
        auto itr = pose_.map.find(jointWeight.first);
        if (itr == pose_.map.end()) {
            continue;
        }
        skinCluster_.inverseBindPoses[itr->second] = jointWeight.second.inverseBindPose;
        for (const auto& vertexWeight : jointWeight.second.weights) {
            auto& currentInfluence = skinCluster_.mappedInfluence[vertexWeight.index];
            for (uint32_t index = 0; index < MAX_INFLUENCE; ++index) {
                if (currentInfluence.weights[index] == 0.f) {
                    currentInfluence.weights[index] = vertexWeight.weight;
                    currentInfluence.jointIndices[index] = static_cast<int32_t>(itr->second);
                    break;
                }
            }
        }
    }
}

void SkinningState::UpdateSkinCluster() {
    for (size_t jointIndex = 0; jointIndex < pose_.joints.size(); ++jointIndex) {
        if (skinCluster_.inverseBindPoses.size() <= jointIndex) {
            Utils::Alert("Joint index out of bounds in skin cluster update");
            break;
        }

        skinCluster_.mappedPalette[jointIndex].space = skinCluster_.inverseBindPoses[jointIndex] * pose_.joints[jointIndex].space;
        skinCluster_.mappedPalette[jointIndex].inverseTranspose = skinCluster_.mappedPalette[jointIndex].space.Inverse().Transpose();
    }
}

void SkinningState::UpdateSkeleton() {
    std::function<void(int32_t)> RecursiveUpdate = [&](int32_t _index) {
        Joint& joint = pose_.joints[_index];
        joint.local = MathUtils::Matrix::MakeAffineMatrix(joint.transform);
        if (joint.parent) {
            joint.space = joint.local * pose_.joints[*joint.parent].space;
        }
        else {
            joint.space = joint.local;
        }

        for (int32_t child : joint.children) {
            RecursiveUpdate(child);
        }
        };

    RecursiveUpdate(pose_.root);
}

void SkinningState::UpdateAnimation() {
    if (!data_->animation.has_value()) {
        Log::Send(Log::Level::WARNING, "Model data does not contain an animation");
        return;
    }
    Animation& animation = data_->animation.value();
    // Update AnimationTimer
    if (animationEnable_) {
        animationTime_ += 1.f / 60.f;
        animationTime_ = fmod(animationTime_, animation.duration);
    }

    // ApplyAnimation
    ApplyAnimation();
}

void SkinningState::ApplyAnimation() {
    if (!data_->animation.has_value()) {
        Log::Send(Log::Level::ERR, "Model data does not contain an animation");
        Utils::Alert("Model data does not contain an animation");
        return;
    }
    Animation& animation = data_->animation.value();

    for (Joint& joint : pose_.joints) {
        if (animation.nodeAnimations.contains(joint.name)) {
            auto& rna = animation.nodeAnimations[joint.name];

            joint.transform.scale = AnimationCurveFunction::Calculate(rna.scale, animationTime_);
            joint.transform.rotate = AnimationCurveFunction::Calculate(rna.rotation, animationTime_);
            joint.transform.translate = AnimationCurveFunction::Calculate(rna.translate, animationTime_);
        }
    }
}

void SkinningState::CreateLine() {
    line_.Clear();

    for (auto& joint : pose_.joints) {
        if (joint.parent.has_value()) {
            line_.AddLine(joint.space.GetTranslate(), pose_.joints[*joint.parent].space.GetTranslate());
        }
    }
    line_.Update();
}
