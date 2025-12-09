#include "Emitter.hpp"

#include "Log.hpp"
#include "Utils.hpp"
#include "imgui_internal.h"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "src/Texture/TextureManager.hpp"

Emitter::Emitter(DirectXAdapter* _adapter, SRVManager* _srv) : adapter_(_adapter), srv_(_srv), handle_() {}

void Emitter::Initialize(const MeshData& _mesh) {
    if (!adapter_) {
        Utils::Alert("Emitter failed to initialize: DirectXAdapter not available");
        throw std::runtime_error("Emitter failed to initialize: DirectXAdapter not available");
    }

    if (!srv_) {
        Utils::Alert("Emitter failed to initialize: SRVManager not available");
        throw std::runtime_error("Emitter failed to initialize: SRVManager not available");
    }

    uuid_ = Utils::GenerateUniqueId();

    data_ = _mesh;

    // Vertex Buffer
    vr_ = adapter_->CreateBufferResource(sizeof(Vertex) * data_.vertices.size());
    vbv_.BufferLocation = vr_->Get()->GetGPUVirtualAddress();
    vbv_.SizeInBytes = static_cast<UINT>(sizeof(Vertex) * data_.vertices.size());
    vbv_.StrideInBytes = sizeof(Vertex);
    Vertex* vd = nullptr;
    vr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&vd));
    vd_ = {vd, data_.vertices.size()};
    std::copy_n(data_.vertices.data(), data_.vertices.size(), vd_.data());

    // Index Buffer
    if (!data_.indices.empty()) {
        ir_ = adapter_->CreateBufferResource(sizeof(uint32_t) * data_.indices.size());
        ibv_.BufferLocation = ir_->Get()->GetGPUVirtualAddress();
        ibv_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * data_.indices.size());
        ibv_.Format = DXGI_FORMAT_R32_UINT;
        uint32_t* id = nullptr;
        ir_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&id));
        id_ = { id, data_.indices.size() };
        std::copy_n(data_.indices.data(), data_.indices.size(), id_.data());
    }

    // Mesh で言うMaterial
    ForGpu* forMap = nullptr;
    resource_ = adapter_->CreateBufferResource(sizeof(ForGpu) * MAX);
    resource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&forMap));
    mapped_ = {forMap, MAX};

    index_ = srv_->Allocate();
    handle_ = srv_->GetGPUHandle(index_);
    srv_->CreateSRVforStructuredBuffer(index_, resource_->Get(), MAX, sizeof(ForGpu));

    Singleton<TextureManager>::GetInstance()->Load(texture_);
}

void Emitter::Update() {
    FrequencyUpdate();

    RegisterGpu();
}

void Emitter::Draw() {
    if (!adapter_) return;
    if (actives_ <= 0) return;

    const auto command = adapter_->GetCommandList();

    command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command->IASetVertexBuffers(0, 1, &vbv_);
    command->SetGraphicsRootDescriptorTable(0, handle_);  // Parameter 0: Particle Buffer
    command->SetGraphicsRootDescriptorTable(1, Singleton<TextureManager>::GetInstance()->GetGPUHandle(texture_));  // Parameter 1: Texture

    if (ir_) {
        command->IASetIndexBuffer(&ibv_);
        command->DrawIndexedInstanced(static_cast<UINT>(id_.size()), actives_, 0, 0, 0);
    }
    else {
        command->DrawInstanced(static_cast<UINT>(vd_.size()), actives_, 0, 0);
    }
}

void Emitter::Emit() {
    Spawn(spawnCount_);
}

void Emitter::Debug() {
    ImGui::PushID(uuid_.c_str());
    if (ImGui::TreeNode((name_ + " (" + uuid_ + ")").c_str())){
        ImGui::Text("Texture: %s", texture_.c_str());
        ImGui::DragFloat3("Position", &position_.x, 0.1f);
        ImGui::DragInt("Spawn Count", reinterpret_cast<int*>(&spawnCount_), 1.f, 1, 1000);
        ImGui::DragFloat("Frequency", &frequency_, 0.01f, 0.f, 100.f);
        ImGui::DragFloat("Duration", &duration_, 0.01f, 0.f, 100.f);
        ImGui::DragFloat3("Size", &size_.x, 0.01f, 0.f, 100.f);
        ImGui::ColorEdit4("Color", &color_.x);

        if (ImGui::TreeNode("Particles")){
            for (const auto& particle : particles_) {
                particle->Debug();
            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
    ImGui::PopID();
}

Emitter& Emitter::SetPosition(const Vector3& _position) {
    position_ = _position;
    return *this;
}
Emitter& Emitter::SetFrequency(const float& _frequency) {
    frequency_ = _frequency;
    return *this;
}
Emitter& Emitter::SetSpawnCount(const uint16_t& _count) {
    spawnCount_ = _count;
    return *this;
}
Emitter& Emitter::SetColor(const Vector4& _color) {
    color_ = _color;
    return *this;
}

Emitter& Emitter::SetTexture(const std::string& _texture) {
    texture_ = _texture;
    Singleton<TextureManager>::GetInstance()->Load(texture_);
    return *this;
}

Emitter& Emitter::SetSize(const float& _size) {
    size_ = {_size, _size, _size};
    return *this;
}

Emitter& Emitter::SetSize(const Vector3& _size) {
    size_ = _size;
    return *this;
}

Emitter& Emitter::SetVelocity(const Vector3& _velocity) {
    velocity_ = _velocity;
    return *this;
}

Emitter& Emitter::SetUpdateFunction(const std::function<void(float, Vector3&, Vector4&)>& _func) {
    particleFunc_ = _func;
    return *this;
}

Emitter& Emitter::SetDuration(const float& _duration) {
    duration_ = _duration;
    return *this;
}

Emitter& Emitter::Enable(bool _active) {
    active_ = _active;
    return *this;
}

void Emitter::FrequencyUpdate() {
    if (!active_) return;
    if (frequency_ <= 0.01f) return;
    if (frequency_ <= timer_) {
        timer_ = 0.f;
        Emit();
        return;
    }

    constexpr float DeltaTime = 1.f / 60.f;
    timer_ += DeltaTime;
}

void Emitter::Spawn(const uint16_t& _count) {
    for (uint16_t i = 0; i < _count; ++i) {
        std::unique_ptr<Particle> particle = std::make_unique<Particle>();
        particle->SetPosition(position_)
            .SetScale(size_)
            .SetColor(color_)
            .SetVelocity(velocity_)
            .SetUpdateFunction(particleFunc_)
            .Initialize(3.f);
        particles_.emplace_back(std::move(particle));
    }
}

void Emitter::RegisterGpu() {
    const auto& ac = Singleton<CameraController>::GetInstance()->GetActive();
    Matrix4x4 billboard = BackToFront * ac->GetMatrix();
    billboard.matrix[3][0] = 0.f;
    billboard.matrix[3][1] = 0.f;
    billboard.matrix[3][2] = 0.f;
    actives_ = 0;

    std::erase_if(particles_, [&](const auto& _p) { return _p->IsDead(); });

    if (particles_.empty())return;

    for (auto& particle : particles_) {
        particle->Update();
        mapped_[actives_].world = MathUtils::Matrix::MakeAffineMatrix(
            MathUtils::Matrix::MakeScaleMatrix(particle->GetScale()),
            billboard,
            MathUtils::Matrix::MakeTranslateMatrix(particle->GetPosition())
        );
        mapped_[actives_].wvp = mapped_[actives_].world * ac->GetViewProjection();
        mapped_[actives_].color = particle->GetColor();
        ++actives_;
    }
}
