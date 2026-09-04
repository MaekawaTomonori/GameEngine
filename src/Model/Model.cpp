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
#include "src/Model/Skinning/SkinningState.hpp"
#include "src/Texture/TextureManager.hpp"

Model::Model() :
    common_(Singleton<ModelCommon>::GetInstance()),
    adapter_(common_->GetAdapter()),
    commandList_(adapter_->GetCommandList()),
    uuid_(Utils::GenerateUniqueId()), transform_() {
}

Model::~Model() {
    common_->Unregister(uuid_);
    common_->UnregisterShadowDraw(uuid_);
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

    // Create SkinningState only if skeleton exists and skinCluster data is available
    if (data_->skeleton.has_value() && !data_->skinCluster.empty()) {
        Log::Send(Log::Level::TRACE, "Creating SkinningState for: " + _name);
        skinning_ = std::make_unique<SkinningState>();
        skinning_->Initialize(adapter_, common_, data_, *mesh_);
        Log::Send(Log::Level::TRACE, "SkinningState created for: " + _name);
    }
    else {
        Log::Send(Log::Level::TRACE, "No valid skinning data found, skipping SkinningState creation for: " + _name);
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

    common_->RegisterUpdate(uuid_, [this](){ UpdateMapData(); });
    common_->RegisterDebug(uuid_, [this](){ Debug(); });
    common_->RegisterShadowDraw(uuid_, [this]() {
        if (!castShadow_) return;
        commandList_->SetGraphicsRootConstantBufferView(0, wr_->Get()->GetGPUVirtualAddress());
        commandList_->SetGraphicsRootConstantBufferView(2, mesh_->GetMaterialAddress());
        commandList_->SetGraphicsRootDescriptorTable(3, Singleton<TextureManager>::GetInstance()->GetGPUHandle(mesh_->GetTextureName()));
        mesh_->DrawGeometryOnly();
    });
}

void Model::Update() {
    // Update Animation, Skeleton, SkinCluster, and debug line only if this model has skinning data
    if (skinning_) {
        skinning_->Update();
    }

    // Mesh Update
    mesh_->Update();
}

void Model::Draw() const {
    if (!commandList_) {
        Log::Send(Log::Level::ERR, "Command list is null");
        return;
    }

    const auto tm = Singleton<TextureManager>::GetInstance();

    const bool isTransparent = mesh_->GetAlpha() < 1.0f;
    if (skinning_) {
        auto drawCmd = [this, tm]() {
            commandList_->SetGraphicsRootConstantBufferView(1, wr_->Get()->GetGPUVirtualAddress());
            commandList_->SetGraphicsRootConstantBufferView(4, cr_->Get()->GetGPUVirtualAddress());
            commandList_->SetGraphicsRootDescriptorTable(8, tm->GetGPUHandle(environmentTexture_));
            commandList_->SetGraphicsRootDescriptorTable(11, skinning_->GetPaletteHandle());
            mesh_->Draw();
        };
        if (isTransparent) {
            common_->RegisterSkinningTransparentDraw(drawCmd, posteffect_);
        } else {
            common_->RegisterSkinningDraw(drawCmd, posteffect_);
        }
    } else {
        auto drawCmd = [this, tm]() {
            commandList_->SetGraphicsRootShaderResourceView(1, wr_->Get()->GetGPUVirtualAddress());
            commandList_->SetGraphicsRootConstantBufferView(4, cr_->Get()->GetGPUVirtualAddress());
            commandList_->SetGraphicsRootDescriptorTable(8, tm->GetGPUHandle(environmentTexture_));
            mesh_->Draw();
        };
        if (isTransparent) {
            common_->RegisterStaticTransparentDraw(drawCmd, posteffect_);
        } else {
            common_->RegisterStaticDraw(drawCmd, posteffect_);
        }
    }

#ifdef _DEBUG
    DrawLine();
#endif
}

Model& Model::SetName(const std::string& _name) {
    name_ = _name;
    return *this;
}

Model& Model::SetTranslate(const Vector3& _translate) {
    transform_.translate = _translate;
    return *this;
}

Model& Model::SetRotate(const Vector3& _rotate) {
    transform_.rotate = _rotate;
    return *this;
}

Model& Model::SetScale(const Vector3& _scale) {
    transform_.scale = _scale;
    return *this;
}

Model& Model::SetEnvironmentTexture(const std::string& _texture) {
    environmentTexture_ = _texture;
    if (!_texture.empty()) {
        const auto tm = Singleton<TextureManager>::GetInstance();
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

Model& Model::SetColor(const Vector4& _color) {
    if (mesh_) {
        mesh_->SetColor(_color);
    }
    return *this;
}

const std::string& Model::GetName() const {
    return name_;
}

GESTD::ReferencePtr<Mesh> Model::GetMesh() const {
    return GESTD::ReferencePtr<Mesh>(mesh_);
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

        if (skinning_) {
            skinning_->Debug(uuid_);
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

void Model::DrawLine() const {
    if (skinning_) {
        skinning_->DrawLine();
    }
}
