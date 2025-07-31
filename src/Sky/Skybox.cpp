#include "Skybox.hpp"

#include "imgui.h"
#include "Math/MathUtils.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Texture/TextureManager.hpp"
#include "src/Camera/Manager/CameraManager.hpp"

void Skybox::Initialize(const std::string& _texture) {
    texture_ = _texture;

    TextureManager* tm = Singleton<TextureManager>::GetInstance();
    tm->Load(texture_);

    common_ = Singleton<SkyCommon>::GetInstance();
    adapter_ = common_->GetAdapter();

    CreateVertex();
    CreateIndex();

    // Transform constant buffer
    wr_ = adapter_->CreateBufferResource(sizeof(Transformation));
    wr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&wd_));

    // Material constant buffer
    mr_ = adapter_->CreateBufferResource(sizeof(Material));
    mr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&md_));
    
    // Initialize material color
    md_->color = {1.0f, 1.0f, 1.0f, 1.0f};

    transform_ = {
        {50.f, 50.f, 50.f},
        Vector3{0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f}
    };
}

void Skybox::Update() {
    float& scale = transform_.scale.x;
    common_->RegisterCommand("Skybox", [&]{
        ImGui::Begin("Skybox");
        ImGui::DragFloat("Scale", &scale, 0.1f);
        ImGui::End();
    });

    transform_.scale = {scale, scale, scale};

    wd_->wvp = MathUtils::Matrix::MakeAffineMatrix(transform_) * Singleton<CameraManager>::GetInstance()->GetActive()->GetViewProjection();
}

void Skybox::Draw() {
    TextureManager* tm = Singleton<TextureManager>::GetInstance();

    // Set vertex buffer
    adapter_->GetCommandList()->IASetVertexBuffers(0, 1, &vbv_);
    adapter_->GetCommandList()->IASetIndexBuffer(&ibv_);
    adapter_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw call from common (sets PSO and root signature)
    common_->Draw();

    // Set constant buffers
    adapter_->GetCommandList()->SetGraphicsRootConstantBufferView(0, mr_->Get()->GetGPUVirtualAddress()); // Material
    adapter_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wr_->Get()->GetGPUVirtualAddress()); // Transform

    // Set texture
    adapter_->GetCommandList()->SetGraphicsRootDescriptorTable(2, tm->GetGPUHandle(texture_));

    // Draw indexed
    adapter_->GetCommandList()->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

void Skybox::CreateVertex() {
    vr_ = adapter_->CreateBufferResource(sizeof(VertexData) * 24);
    vbv_.BufferLocation = vr_->Get()->GetGPUVirtualAddress();
    vbv_.SizeInBytes = sizeof(VertexData) * 24;
    vbv_.StrideInBytes = sizeof(VertexData);

    vr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&vd_));

    vd_[0].position = { 1.f, 1.f, 1.f, 1.f };
    vd_[1].position = { 1.f, 1.f, -1.f, 1.f };
    vd_[2].position = { 1.f, -1.f, 1.f, 1.f };
    vd_[3].position = { 1.f, -1.f, -1.f, 1.f };

    vd_[4].position = { -1.f, 1.f, 1.f, 1.f };
    vd_[5].position = { -1.f, 1.f, -1.f, 1.f };
    vd_[6].position = { -1.f, -1.f, 1.f, 1.f };
    vd_[7].position = { -1.f, -1.f, -1.f, 1.f };

    vd_[8].position = { -1.f, 1.f, 1.f, 1.f };
    vd_[9].position = { 1.f, 1.f, 1.f, 1.f };
    vd_[10].position = { -1.f, -1.f, 1.f, 1.f };
    vd_[11].position = { 1.f, -1.f, 1.f, 1.f };

    vd_[12].position = { -1.f, 1.f, -1.f, 1.f };
    vd_[13].position = { 1.f, 1.f, -1.f, 1.f };
    vd_[14].position = { -1.f, -1.f, -1.f, 1.f };
    vd_[15].position = { 1.f, -1.f, -1.f, 1.f };

    vd_[16].position = { -1.f, 1.f, -1.f, 1.f };
    vd_[17].position = { 1.f, 1.f, -1.f, 1.f };
    vd_[18].position = { -1.f, 1.f, 1.f, 1.f };
    vd_[19].position = { 1.f, 1.f, 1.f, 1.f };

    vd_[20].position = { -1.f, -1.f, 1.f, 1.f };
    vd_[21].position = { 1.f, -1.f, 1.f, 1.f };
    vd_[22].position = { -1.f, -1.f, -1.f, 1.f };
    vd_[23].position = { 1.f, -1.f, -1.f, 1.f };
}

void Skybox::CreateIndex() {
    ir_ = adapter_->CreateBufferResource(sizeof(uint32_t) * 36);
    ibv_.BufferLocation = ir_->Get()->GetGPUVirtualAddress();
    ibv_.SizeInBytes = sizeof(uint32_t) * 36;
    ibv_.Format = DXGI_FORMAT_R32_UINT;

    ir_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&id_));

    // Right face (0-3)
    id_[0] = 0; id_[1] = 1; id_[2] = 2;
    id_[3] = 1; id_[4] = 3; id_[5] = 2;

    // Left face (4-7)
    id_[6] = 4; id_[7] = 5; id_[8] = 6;
    id_[9] = 5; id_[10] = 7; id_[11] = 6;

    // Front face (8-11)
    id_[12] = 8; id_[13] = 9; id_[14] = 10;
    id_[15] = 9; id_[16] = 11; id_[17] = 10;

    // Back face (12-15)
    id_[18] = 12; id_[19] = 13; id_[20] = 14;
    id_[21] = 13; id_[22] = 15; id_[23] = 14;

    // Top face (16-19)
    id_[24] = 16; id_[25] = 17; id_[26] = 18;
    id_[27] = 17; id_[28] = 19; id_[29] = 18;

    // Bottom face (20-23)
    id_[30] = 20; id_[31] = 21; id_[32] = 22;
    id_[33] = 21; id_[34] = 23; id_[35] = 22;
}
