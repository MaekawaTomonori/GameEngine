#include "Mesh.hpp"

#include <filesystem>

#include "imgui.h"
#include "DebugUI.hpp"

#include "Math/MathUtils.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Light/LightManager.hpp"
#include "src/Texture/TextureManager.hpp"

void Mesh::Initialize(DirectXAdapter* _adapter, const std::string &_name, const MeshData& _raw) {
    adapter_ = _adapter;
    commandList_ = adapter_->GetCommandList();
    name_ = _name;
    data_ = _raw;

    vr_ = adapter_->CreateBufferResource(sizeof(Vertex) * data_.vertices.size());

    vbv_.BufferLocation = vr_->Get()->GetGPUVirtualAddress();
    vbv_.SizeInBytes = static_cast<UINT>(sizeof(Vertex) * data_.vertices.size());
    vbv_.StrideInBytes = sizeof(Vertex);

    vr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&vd_));
    std::copy_n(data_.vertices.data(), data_.vertices.size(), vd_);

    vbvs_.push_back(vbv_);

    mr_ = adapter_->CreateBufferResource(sizeof(Material));
    mr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&material_));

    material_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    material_->lighting = 0; // Default lighting
    material_->shininess = 100.f;
    material_->coefficient = 0.0f; // Environment mapping coefficient (disabled by _default)
    material_->tilingMul = Vector2(1.0f, 1.0f);
    material_->uvTransform = MathUtils::Matrix::MakeIdentity();

    if (!data_.indices.empty()){
        ir_ = adapter_->CreateBufferResource(sizeof(uint32_t) * data_.indices.size());
        ibv_.BufferLocation = ir_->Get()->GetGPUVirtualAddress();
        ibv_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * data_.indices.size());
        ibv_.Format = DXGI_FORMAT_R32_UINT;

        ir_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&id_));
        std::copy_n(data_.indices.data(), data_.indices.size(), id_);
    }

    Singleton<TextureManager>::GetInstance()->Load(data_.texture);
    texture_ = data_.texture;

    lighting_ = true;
}

void Mesh::Update() {
    if (material_) {
        material_->uvTransform = MathUtils::Matrix::MakeIdentity() * MathUtils::Matrix::MakeScaleMatrix({ material_->tilingMul.x, material_->tilingMul.y, 1.0f });
    }
}

void Mesh::Draw(const uint16_t _instanceCount) const {
    if (!commandList_)return;

    commandList_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList_->IASetVertexBuffers(0, static_cast<UINT>(vbvs_.size()), vbvs_.data());

    if (!data_.indices.empty()){
        commandList_->IASetIndexBuffer(&ibv_);
    }
    commandList_->SetGraphicsRootConstantBufferView(0, mr_->Get()->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootDescriptorTable(2, Singleton<TextureManager>::GetInstance()->GetGPUHandle(texture_));

    if (lighting_) {
        material_->lighting = 1;
        Singleton<LightManager>::GetInstance()->Draw();
    }
    !data_.indices.empty() ? commandList_->DrawIndexedInstanced(static_cast<UINT>(data_.indices.size()), _instanceCount, 0, 0, 0) : commandList_->DrawInstanced(static_cast<UINT>(data_.vertices.size()), _instanceCount, 0, 0);
}

void Mesh::Debug() {
    ImGui::ColorEdit4("Color", &material_->color.x);
    ImGui::Checkbox("EnableLighting", &lighting_);
    if (lighting_) {
        ImGui::DragFloat("Shininess", &material_->shininess, 0.1f, 0.f, 100.f);
    } else{
        material_->lighting = 0;
    }
    ImGui::DragFloat("Environment Coefficient", &material_->coefficient, 0.01f, 0.0f, 1.0f);

    ImGui::Separator();
    ImGui::Text("UV Settings");

    ImGui::Checkbox("Lock Aspect Ratio", &aspectRatioLocked_);

    if (aspectRatioLocked_) {
        // Store previous values to detect changes
        static float prevX = material_->tilingMul.x;
        static float prevY = material_->tilingMul.y;

        // Update aspect ratio when first locking
        if (ImGui::IsItemActivated()) {
            if (material_->tilingMul.y != 0.0f) {
                aspectRatio_ = material_->tilingMul.x / material_->tilingMul.y;
            }
            prevX = material_->tilingMul.x;
            prevY = material_->tilingMul.y;
        }

        if (ImGui::DragFloat("Tiling X", &material_->tilingMul.x, 0.1f, 0.1f, 10.0f)) {
            // X changed, update Y to maintain aspect ratio
            if (material_->tilingMul.x != prevX) {
                material_->tilingMul.y = material_->tilingMul.x / aspectRatio_;
                prevX = material_->tilingMul.x;
                prevY = material_->tilingMul.y;
            }
        }

        if (ImGui::DragFloat("Tiling Y", &material_->tilingMul.y, 0.1f, 0.1f, 10.0f)) {
            // Y changed, update X to maintain aspect ratio
            if (material_->tilingMul.y != prevY) {
                material_->tilingMul.x = material_->tilingMul.y * aspectRatio_;
                prevX = material_->tilingMul.x;
                prevY = material_->tilingMul.y;
            }
        }
    } else {
        ImGui::DragFloat2("Tiling Multiplier", &material_->tilingMul.x, 0.1f, 0.1f, 10.0f);
    }

    ImGui::Text("UV Transform");
    if (ImGui::TreeNode("UV Transform Matrix")) {
        ImGui::DragFloat4("Row 1", &material_->uvTransform.matrix[0][0], 0.01f);
        ImGui::DragFloat4("Row 2", &material_->uvTransform.matrix[1][0], 0.01f);
        ImGui::DragFloat4("Row 3", &material_->uvTransform.matrix[2][0], 0.01f);
        ImGui::DragFloat4("Row 4", &material_->uvTransform.matrix[3][0], 0.01f);

        if (ImGui::Button("Reset to Identity")) {
            material_->uvTransform = MathUtils::Matrix::MakeIdentity();
        }
        ImGui::TreePop();
    }
}

void Mesh::SetVBV(const D3D12_VERTEX_BUFFER_VIEW _vbv) {
    vbvs_.push_back(_vbv);
}

MeshData Mesh::GetData() const {
    return data_;
}

void Mesh::SetTexture(const std::string& _texturePath) {
    // Load the new texture
    Singleton<TextureManager>::GetInstance()->Load(_texturePath);

    // Update the current texture path
    texture_ = _texturePath;
}

void Mesh::SetTextureSize(const Vector2 _tilingMul) const {
    if (material_) {
        material_->tilingMul = _tilingMul;
    }
}

void Mesh::SetColor(const Vector4& _color) const {
    material_->color = _color;
}

void Mesh::EnableLighting(bool _active) {
    lighting_ = _active;
}
