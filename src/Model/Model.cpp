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
#include "src/Camera/Controller/CameraController.hpp"
#include "src/Texture/TextureManager.hpp"

Model::Model() :
    common_(Singleton<ModelCommon>::GetInstance()),
    adapter_(common_->GetAdapter()),
    commandList_(adapter_->GetCommandList()),
    uuid_(Utils::GenerateUniqueId()), transform_() {
}

Model::~Model() {
    common_->Unregister(uuid_);
}

void Model::Initialize(const std::string& _name) {
    if (!adapter_) {
        Log::Send(Log::Level::ERR, "Adapter is null");
        return;
    }
    Log::Send(Log::Level::INFO, "[Model] Initialize: " + _name);

    Load(_name);

    Log::Send(Log::Level::INFO, "[Model] data loaded: " + _name);

    data_ = common_->GetResourceRepository()->GetModelRepository()->Get(_name);

    // IsNull
    if (!data_) {
        Log::Send(Log::Level::ERR, "[Model] data is null for: " + _name);
        Utils::Alert("Model data is null for: " + _name);
        return;
    }

    Log::Send(Log::Level::INFO, "[Model] Creating Mesh: " + _name);

    mesh_ = std::make_unique<Mesh>();
    mesh_->Initialize(adapter_, _name, common_->GetResourceRepository()->GetMeshRepository()->Get(data_->mesh));

    Log::Send(Log::Level::INFO, "[Model] Mesh created: " + _name);

    wr_ = (adapter_->CreateBufferResource(sizeof(Transformation)));
    wr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&wd_));
    wd_->wvp = MathUtils::Matrix::MakeIdentity();
    wd_->world = MathUtils::Matrix::MakeIdentity();
    wd_->inverse = MathUtils::Matrix::MakeIdentity();

    cr_ = (adapter_->CreateBufferResource(sizeof(CameraForGpu)));
    cr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&cd_));

    // Copy skeleton as per-instance pose
    if (data_->skeleton.has_value()) {
        pose_ = data_->skeleton.value();
    }

    // Create SkinCluster only if skeleton exists and skinCluster data is available
    if (pose_.has_value() && !data_->skinCluster.empty()) {
        Log::Send(Log::Level::TRACE, "Creating SkinCluster for: " + _name);
        CreateSkinCluster();
        Log::Send(Log::Level::TRACE, "SkinCluster created for: " + _name);
    }
    else {
        Log::Send(Log::Level::TRACE, "No valid skinning data found, skipping SkinCluster creation for: " + _name);
    }


    transform_ = {
        {1,1,1},
        Vector3{0,0,0},
        {0,0,0},
    };

    // Set default environment texture if none specified
    if (environmentTexture_.empty()) {
        environmentTexture_ = "white_x16.png";
        Singleton<TextureManager>::GetInstance()->Load(environmentTexture_);
    }

    line_.Initialize();

    common_->RegisterUpdate(uuid_, [this](){ UpdateMapData(); });
    common_->RegisterDebug(uuid_, [this](){ Debug(); });
}

void Model::Update() {
    // Update Animation, Skeleton, and SkinCluster only if valid skinning data exists
    if (pose_.has_value() && !data_->skinCluster.empty()) {
        UpdateAnimation();
        UpdateSkeleton();
        UpdateSkinCluster();
    }

    // Joint to Line (only if valid skinning data exists)
    if (pose_.has_value() && !data_->skinCluster.empty()) {
        CreateLine();
    }

    // Mesh Update
    mesh_->Update();
}

void Model::Draw() const {
    if (!commandList_) {
        Log::Send(Log::Level::ERR, "Command list is null");
        return;
    }

    TextureManager* tm = Singleton<TextureManager>::GetInstance();

    if (pose_.has_value() && !data_->skinCluster.empty()) {
        common_->RegisterSkinningDraw([this, tm]() {
            commandList_->SetGraphicsRootConstantBufferView(1, wr_->Get()->GetGPUVirtualAddress());
            commandList_->SetGraphicsRootConstantBufferView(4, cr_->Get()->GetGPUVirtualAddress());
            commandList_->SetGraphicsRootDescriptorTable(8, tm->GetGPUHandle(environmentTexture_));
            commandList_->SetGraphicsRootDescriptorTable(9, skinCluster_.paletteHandle.second);
            mesh_->Draw();
            }, posteffect_);
    }else {
        common_->RegisterStaticDraw([this, tm]() {
            commandList_->SetGraphicsRootConstantBufferView(1, wr_->Get()->GetGPUVirtualAddress());
            commandList_->SetGraphicsRootConstantBufferView(4, cr_->Get()->GetGPUVirtualAddress());
            commandList_->SetGraphicsRootDescriptorTable(8, tm->GetGPUHandle(environmentTexture_));
            mesh_->Draw();
            }, posteffect_);
    }

    DrawLine();
}

Model& Model::SetName(const std::string& _name) {
    name_ = _name;
    return *this;
}

Model& Model::SetTranslate(const Vector3 _translate) {
    transform_.translate = _translate;
    return *this;
}

Model& Model::SetRotate(const Vector3 _rotate) {
    transform_.rotate = _rotate;
    return *this;
}

Model& Model::SetScale(const Vector3 _scale) {
    transform_.scale = _scale;
    return *this;
}

Model& Model::SetEnvironmentTexture(const std::string& _texture) {
    environmentTexture_ = _texture;
    if (!_texture.empty()) {
        TextureManager* tm = Singleton<TextureManager>::GetInstance();
        tm->Load(_texture);
    }
    return *this;
}

Model& Model::SetTexture(const std::string& _texture) {
    if (mesh_) {
        mesh_->SetTexture(_texture);
    }
    else {
        Log::Send(Log::Level::WARNING, "Mesh is not initialized. Cannot set texture.");
    }
    return *this;
}

Model& Model::SetTilingMultiply(const Vector2 _mul) {
    if (mesh_) {
        mesh_->SetTextureSize(_mul);
    }
    else {
        Log::Send(Log::Level::WARNING, "Mesh is not initialized. Cannot set tiling multiplier.");
    }
    return *this;
}

Model& Model::SetColor(const Vector4 _color) {
    if (mesh_) {
        mesh_->SetColor(_color);
    }
    return *this;
}

const std::string& Model::GetName() const {
    return name_;
}

Mesh* Model::GetMesh() const {
    return mesh_.get();
}

void Model::Load(const std::string& _name) {
    auto repo = Singleton<ModelCommon>::GetInstance()->GetResourceRepository();
    if (repo->GetModelRepository()->Contains(_name)){
        Log::Send(Log::Level::WARNING, "Already Loaded Model : " + _name);
        return;
    }

    std::unique_ptr<IModelLoader> loader;
    if (std::filesystem::exists("Assets/Resources/" + _name + "/" + _name + ".obj")) {
        loader = std::make_unique<ObjLoader>();
    }
    else if (std::filesystem::exists("Assets/Resources/" + _name + "/" + _name + ".gltf")) {
        loader = std::make_unique<GltfLoader>();
    }
    else {
        Log::Send(Log::Level::ERR, "Model::Load: Model not found: " + _name);
        Utils::Alert("Model not found: " + _name);
        return;
    }
    loader->LoadModel(_name, repo);
}

void Model::Debug() {
    const std::string& label = name_.empty() ? data_->name : name_;
    ImGui::PushID(uuid_.c_str());
    if (ImGui::CollapsingHeader(label.c_str())) {
        ImGui::SeparatorText("Model Info");
        if (ImGui::TreeNode("Transform")) {
            ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);
            ImGui::DragFloat3("Rotate", &std::get<Vector3>(transform_.rotate).x, 0.1f);
            ImGui::DragFloat3("Position", &transform_.translate.x, 0.1f);
            if (ImGui::Button("Reset Transform")) {
                transform_ = Transform{
                    {1, 1, 1},
                    Vector3{0, 0, 0},
                    {0, 0, 0},
                };
            }
            ImGui::TreePop();
        }
        if (pose_.has_value()) {
            ImGui::PushID(("skeleton_" + uuid_).c_str());
            Skeleton& skeleton = pose_.value();
            ImGui::SeparatorText("Skeleton");
            std::function<void(int32_t)> Recursive = [&](int32_t _index) {
                Joint& joint = skeleton.joints[_index];
                if (ImGui::TreeNode(joint.name.c_str())) {
                    ImGui::Text("Joint: %s", joint.name.c_str());
                    ImGui::Text("Index: %d", joint.index);
                    ImGui::Text("Parent: %s", joint.parent.has_value() ? skeleton.joints[joint.parent.value()].name.c_str() : "None");
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
            Recursive(skeleton.root);
            ImGui::PopID();
        }

        if (data_->animation.has_value()) {
            ImGui::SeparatorText("Animation");
            if (ImGui::TreeNode("Details")) {
                ImGui::Checkbox("Enable", &animationEnable_);
                ImGui::SameLine();
                ImGui::Checkbox("TimerLock", &animationTimerLock_);
                if (animationTimerLock_)ImGui::Text("Timer : %f", animationTime_);
                else ImGui::DragFloat("Anime Timer", &animationTime_);
                ImGui::TreePop();
            }
        }

        ImGui::SeparatorText("Mesh");
        if (ImGui::TreeNode("Mesh")) {
            mesh_->Debug();
            ImGui::TreePop();
        }
    }
    ImGui::PopID();
}

void Model::UpdateMapData() const {
    auto camera = Singleton<CameraController>::GetInstance()->GetActive();

    wd_->world = MathUtils::Matrix::MakeAffineMatrix(transform_.scale, std::get<Vector3>(transform_.rotate), transform_.translate);
    wd_->wvp = wd_->world * camera->GetViewProjection();
    wd_->inverse = wd_->world.Inverse().Transpose();

    *cd_ = camera->GetCameraForGpu();
}

void Model::CreateSkinCluster() {
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

    if (!pose_.has_value() || data_->skinCluster.empty()) {
        Log::Send(Log::Level::WARNING, "Model data does not contain valid skinning _data, skipping skin cluster creation");
        return;
    }

    Skeleton& skeleton = pose_.value();

    size_t jointSize = skeleton.joints.size();
    size_t verticesSize = mesh_->GetData().vertices.size();

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

    SetBindPose(skeleton);

    mesh_->SetVBV(skinCluster_.influenceBufferView);
}

void Model::SetBindPose(Skeleton& _skeleton) {
    size_t jointSize = _skeleton.joints.size();
    skinCluster_.inverseBindPoses.resize(jointSize);
    for (auto& inverseBindPose : skinCluster_.inverseBindPoses) {
        inverseBindPose = MathUtils::Matrix::MakeIdentity();
    }

    for (const auto& jointWeight : data_->skinCluster) {
        auto itr = _skeleton.map.find(jointWeight.first);
        if (itr == _skeleton.map.end()) {
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

void Model::UpdateSkinCluster() {
    if (!pose_.has_value() || data_->skinCluster.empty()) {
        Log::Send(Log::Level::WARNING, "Model data does not contain valid skinning _data, skipping skin cluster update");
        return;
    }

    Skeleton& skeleton = pose_.value();
    for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
        if (skinCluster_.inverseBindPoses.size() <= jointIndex) {
            Utils::Alert("Joint index out of bounds in skin cluster update");
            break;
        }

        skinCluster_.mappedPalette[jointIndex].space = skinCluster_.inverseBindPoses[jointIndex] * skeleton.joints[jointIndex].space;
        skinCluster_.mappedPalette[jointIndex].inverseTranspose = skinCluster_.mappedPalette[jointIndex].space.Inverse().Transpose();
    }
}

void Model::UpdateSkeleton() {
    if (!pose_.has_value() || data_->skinCluster.empty()) {
        Log::Send(Log::Level::WARNING, "Model data does not contain valid skinning _data, skipping skeleton update");
        return;
    }

    Skeleton& skeleton = pose_.value();
    std::function<void(int32_t)> RecursiveUpdate = [&](int32_t _index) {
        Joint& joint = skeleton.joints[_index];
        joint.local = MathUtils::Matrix::MakeAffineMatrix(joint.transform);
        if (joint.parent) {
            joint.space = joint.local * skeleton.joints[*joint.parent].space;
        }
        else {
            joint.space = joint.local;
        }

        for (int32_t child : joint.children) {
            RecursiveUpdate(child);
        }
        };

    RecursiveUpdate(skeleton.root);
}

void Model::UpdateAnimation() {
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

void Model::ApplyAnimation() {
    if (!pose_.has_value() || data_->skinCluster.empty()) {
        Log::Send(Log::Level::WARNING, "Model data does not contain valid skinning _data, skipping animation application");
        return;
    }
    Skeleton& skeleton = pose_.value();

    if (!data_->animation.has_value()) {
        Log::Send(Log::Level::ERR, "Model data does not contain an animation");
        Utils::Alert("Model data does not contain an animation");
        return;
    }
    Animation& animation = data_->animation.value();

    for (Joint& joint : skeleton.joints) {
        if (animation.nodeAnimations.contains(joint.name)) {
            auto& rna = animation.nodeAnimations[joint.name];

            joint.transform.scale = AnimationCurveFunction::Calculate(rna.scale, animationTime_);
            joint.transform.rotate = AnimationCurveFunction::Calculate(rna.rotation, animationTime_);
            joint.transform.translate = AnimationCurveFunction::Calculate(rna.translate, animationTime_);
        }
    }
}

void Model::CreateLine() {
    line_.Clear();

    if (!pose_.has_value() || data_->skinCluster.empty()) {
        Log::Send(Log::Level::WARNING, "Model data does not contain valid skinning _data, skipping line creation");
        return;
    }
    Skeleton& skeleton = pose_.value();

    for (auto& joint : skeleton.joints) {
        if (joint.parent.has_value()) {
            line_.AddLine(joint.space.GetTranslate(), skeleton.joints[*joint.parent].space.GetTranslate());
        }
    }
    line_.Update();
}

void Model::DrawLine() const {
    line_.Draw();
}
