#include "ModelInstance.hpp"

#include "Log.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "Pattern/Singleton.hpp"
#include "Loader/ModelLoaderFactory.hpp"
#include "Math/MathUtils.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "src/Model/Common/ModelCommon.hpp"
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

void ModelInstance::InitializeCommon(const std::string& _name) {
    if (!adapter_) {
        Log::Send(Log::Level::ERR, "Adapter is null");
        return;
    }
    Log::Send(Log::Level::INFO, "[ModelInstance] Initialize: " + _name);

    ModelLoaderFactory::Load(_name, common_->GetResourceRepository());

    data_ = common_->GetResourceRepository()->GetModelRepository()->Get(_name);

    if (!data_) {
        Log::Send(Log::Level::ERR, "[ModelInstance] data is null for: " + _name);
        Utils::Alert("Model data is null for: " + _name);
        return;
    }

    mesh_ = std::make_unique<Mesh>();
    mesh_->Initialize(adapter_, _name, common_->GetResourceRepository()->GetMeshRepository()->Get(data_->mesh));

    wr_ = (adapter_->CreateBufferResource(sizeof(Transformation)));
    wr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&wd_));
    wd_->wvp = MathUtils::Matrix::MakeIdentity();
    wd_->world = MathUtils::Matrix::MakeIdentity();
    wd_->inverse = MathUtils::Matrix::MakeIdentity();

    transform_ = {
        {1,1,1},
        Vector3{0,0,0},
        {0,0,0},
    };

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

void ModelInstance::UpdateMapData() const {
    auto camera = Singleton<CameraController>::GetInstance()->GetActive();

    wd_->world = MathUtils::Matrix::MakeAffineMatrix(transform_.scale, std::get<Vector3>(transform_.rotate), transform_.translate);
    wd_->wvp = wd_->world * camera->GetViewProjection();
    wd_->inverse = wd_->world.Inverse().Transpose();
}

void ModelInstance::DebugTransformSection() {
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
}

void ModelInstance::DebugMeshSection() {
    ImGui::SeparatorText("Mesh");
    if (ImGui::TreeNode("Mesh")) {
        mesh_->Debug();
        ImGui::TreePop();
    }
}
