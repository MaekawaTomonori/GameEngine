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

    vr_ = std::make_unique<DX12Resource>();
    vr_->Create(adapter_->CreateBufferResource(sizeof(Vertex) * data_.vertices.size()));

    vbv_.BufferLocation = vr_->Get()->GetGPUVirtualAddress();
    vbv_.SizeInBytes = static_cast<UINT>(sizeof(Vertex) * data_.vertices.size());
    vbv_.StrideInBytes = sizeof(Vertex);

    vr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&vd_));
    std::copy_n(data_.vertices.data(), data_.vertices.size(), vd_);

    vbvs_.push_back(vbv_);

    mr_ = std::make_unique<DX12Resource>();
    mr_->Create(adapter_->CreateBufferResource(sizeof(Material)));
    mr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&material_));

    material_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    material_->lighting = 0; // Default lighting
    material_->shininess = 100.f;

    if (!data_.indices.empty()){
        ir_ = std::make_unique<DX12Resource>();
        ir_->Create(adapter_->CreateBufferResource(sizeof(uint32_t) * data_.indices.size()));
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
}

void Mesh::Draw() const {
    if (!commandList_)return;

    commandList_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList_->IASetVertexBuffers(0, static_cast<UINT>(vbvs_.size()), vbvs_.data());

    if (!data_.indices.empty()){
        commandList_->IASetIndexBuffer(&ibv_);
    }
    commandList_->SetGraphicsRootConstantBufferView(0, mr_->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootDescriptorTable(2, Singleton<TextureManager>::GetInstance()->GetGPUHandle(texture_));

    if (lighting_) {
        material_->lighting = 1;
        Singleton<LightManager>::GetInstance()->Draw();
    }
    if (!data_.indices.empty()){
        commandList_->DrawIndexedInstanced(static_cast<UINT>(data_.indices.size()), 1, 0, 0, 0);
    } else {
        commandList_->DrawInstanced(static_cast<UINT>(data_.vertices.size()), 1, 0, 0);
    }
}

void Mesh::Debug() {
    ImGui::ColorEdit4("Color", &material_->color.x);
    ImGui::Checkbox("EnableLighting", &lighting_);
    if (lighting_) {
        ImGui::DragFloat("Shininess", &material_->shininess, 0.1f, 0.f, 100.f);
    } else{
        material_->lighting = 0;
    }
}

void Mesh::SetVBV(const D3D12_VERTEX_BUFFER_VIEW _vbv) {
    vbvs_.push_back(_vbv);
}

MeshData Mesh::GetData() const {
    return data_;
}
