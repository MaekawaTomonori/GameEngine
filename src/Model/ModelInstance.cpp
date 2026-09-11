#include "ModelInstance.hpp"

#include "Log.hpp"
#include "PerformanceProfiler.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "Pattern/Singleton.hpp"
#include "Loader/ModelLoaderFactory.hpp"
#include "Math/MathUtils.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/Model/Skinning/SkinningState.hpp"
#include "src/Texture/TextureManager.hpp"

ModelInstance::ModelInstance() :
    common_(Singleton<ModelCommon>::GetInstance()),
    adapter_(common_->GetAdapter()),
    commandList_(adapter_->GetCommandList()),
    uuid_(Utils::GenerateUniqueId()), transform_() {
}

ModelInstance::~ModelInstance() {
    common_->Unregister(uuid_);
    common_->UnregisterShadowDraw(uuid_);
}

void ModelInstance::Initialize(const std::string& _name) {
    if (!adapter_) {
        Log::Send(Log::Level::ERR, "Adapter is null");
        return;
    }
    Log::Send(Log::Level::INFO, "[ModelInstance] Initialize: " + _name);

    ModelLoaderFactory::Load(_name, common_->GetResourceRepository());

    Log::Send(Log::Level::INFO, "[ModelInstance] data loaded: " + _name);

    data_ = common_->GetResourceRepository()->GetModelRepository()->Get(_name);

    // IsNull
    if (!data_) {
        Log::Send(Log::Level::ERR, "[ModelInstance] data is null for: " + _name);
        Utils::Alert("Model data is null for: " + _name);
        return;
    }

    Log::Send(Log::Level::INFO, "[ModelInstance] Creating Mesh: " + _name);

    mesh_ = std::make_unique<Mesh>();
    mesh_->Initialize(adapter_, _name, common_->GetResourceRepository()->GetMeshRepository()->Get(data_->mesh));

    Log::Send(Log::Level::INFO, "[ModelInstance] Mesh created: " + _name);

    wr_ = (adapter_->CreateBufferResource(sizeof(Transformation)));
    wr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&wd_));
    wd_->wvp = MathUtils::Matrix::MakeIdentity();
    wd_->world = MathUtils::Matrix::MakeIdentity();
    wd_->inverse = MathUtils::Matrix::MakeIdentity();

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

    if (skinning_) {
        drawCommand_ = [this]() {
            const auto tm = Singleton<TextureManager>::GetInstance();
            commandList_->SetGraphicsRootConstantBufferView(1, wr_->Get()->GetGPUVirtualAddress());
            commandList_->SetGraphicsRootConstantBufferView(4, common_->GetCameraCBVAddress());
            commandList_->SetGraphicsRootDescriptorTable(8, tm->GetGPUHandle(environmentTexture_));
            commandList_->SetGraphicsRootDescriptorTable(11, skinning_->GetPaletteHandle());
            mesh_->Draw();
        };
    } else {
        drawCommand_ = [this]() {
            const auto tm = Singleton<TextureManager>::GetInstance();
            commandList_->SetGraphicsRootShaderResourceView(1, wr_->Get()->GetGPUVirtualAddress());
            commandList_->SetGraphicsRootConstantBufferView(4, common_->GetCameraCBVAddress());
            commandList_->SetGraphicsRootDescriptorTable(8, tm->GetGPUHandle(environmentTexture_));
            mesh_->Draw();
        };
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
        commandList_->SetGraphicsRootDescriptorTable(3, Singleton<TextureManager>::GetInstance()->GetGPUHandle(mesh_->GetTextureSrvIndex()));
        mesh_->DrawGeometryOnly();
    });
}

void ModelInstance::Update() {
    // Update Animation, Skeleton, SkinCluster, and debug line only if this model has skinning data
    if (skinning_) {
        PROFILE_SCOPE("Model - Skinning");
        skinning_->Update();
    }

    // Mesh Update
    { PROFILE_SCOPE("Model - Mesh"); mesh_->Update(); }
}

void ModelInstance::Draw() const {
    if (!commandList_) {
        Log::Send(Log::Level::ERR, "Command list is null");
        return;
    }

    const bool isTransparent = mesh_->GetAlpha() < 1.0f;
    if (skinning_) {
        if (isTransparent) {
            common_->RegisterSkinningTransparentDraw(drawCommand_, canvasName_);
        } else {
            common_->RegisterSkinningDraw(drawCommand_, canvasName_);
        }
    } else {
        if (isTransparent) {
            common_->RegisterStaticTransparentDraw(drawCommand_, canvasName_);
        } else {
            common_->RegisterStaticDraw(drawCommand_, canvasName_);
        }
    }

#ifdef _DEBUG
    DrawLine();
#endif
}

ModelInstance& ModelInstance::SetName(const std::string& _name) {
    name_ = _name;
    return *this;
}

ModelInstance& ModelInstance::SetTranslate(const Vector3& _translate) {
    transform_.translate = _translate;
    return *this;
}

ModelInstance& ModelInstance::SetRotate(const Vector3& _rotate) {
    transform_.rotate = _rotate;
    return *this;
}

ModelInstance& ModelInstance::SetScale(const Vector3& _scale) {
    transform_.scale = _scale;
    return *this;
}

ModelInstance& ModelInstance::SetEnvironmentTexture(const std::string& _texture) {
    environmentTexture_ = _texture;
    if (!_texture.empty()) {
        const auto tm = Singleton<TextureManager>::GetInstance();
        tm->Load(_texture);
    }
    return *this;
}

ModelInstance& ModelInstance::SetTexture(const std::string& _texture) {
    if (mesh_) {
        mesh_->SetTexture(_texture);
    }
    else {
        Log::Send(Log::Level::WARNING, "Mesh is not initialized. Cannot set texture.");
    }
    return *this;
}

ModelInstance& ModelInstance::SetTilingMultiply(const Vector2 _mul) {
    if (mesh_) {
        mesh_->SetTextureSize(_mul);
    }
    else {
        Log::Send(Log::Level::WARNING, "Mesh is not initialized. Cannot set tiling multiplier.");
    }
    return *this;
}

ModelInstance& ModelInstance::SetColor(const Vector4& _color) {
    if (mesh_) {
        mesh_->SetColor(_color);
    }
    return *this;
}

ModelInstance& ModelInstance::SetCanvasName(const std::string& _canvasName) {
    canvasName_ = _canvasName;
    return *this;
}

const std::string& ModelInstance::GetName() const {
    return name_;
}

void ModelInstance::Debug() {
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

void ModelInstance::UpdateMapData() const {
    auto camera = Singleton<CameraController>::GetInstance()->GetActive();

    wd_->world = MathUtils::Matrix::MakeAffineMatrix(transform_.scale, std::get<Vector3>(transform_.rotate), transform_.translate);
    wd_->wvp = wd_->world * camera->GetViewProjection();
    wd_->inverse = wd_->world.Inverse().Transpose();
}

void ModelInstance::DrawLine() const {
    if (skinning_) {
        skinning_->DrawLine();
    }
}
