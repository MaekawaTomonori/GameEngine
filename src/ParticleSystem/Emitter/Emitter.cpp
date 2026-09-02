#include "Emitter.hpp"

#include "DebugUIWidgets.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "imgui_internal.h"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "src/Texture/TextureManager.hpp"
#include "src/Time/Time.hpp"

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
    srv_->CreateSRVForStructuredBuffer(index_, resource_->Get(), MAX, sizeof(ForGpu));

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
    active_ = true;
    timer_ = 0.f;
    elapsedTime_ = 0.f;
    Spawn(spawnCount_);

    // frequency<=0 は継続スポーンを行わない単発バーストとして扱う
    if (frequency_ <= 0.01f) {
        active_ = false;
    }
}

void Emitter::Reset() {
    particles_.clear();
    timer_ = 0.f;
    elapsedTime_ = 0.f;
    actives_ = 0;
    active_ = false;
    billboard_ = true;
    primitive_ = PrimitiveType::Billboard;
    position_ = {};
    rotation_ = {};
    rotationVelocity_ = {};
    duration_ = 1.f;
    frequency_ = 0.f;
    spawnCount_ = 1;
    size_ = { 1.f, 1.f, 1.f };
    velocity_ = {};
    color_ = { 1.f, 1.f, 1.f, 1.f };
    colorKeys_.clear();
    sizeKeys_.clear();
    particleLifetime_ = 3.f;
    texture_ = "white_x16.png";
    updateFunc_ = nullptr;
    spawnFunc_ = nullptr;
}

bool Emitter::IsFinished() const {
    return particles_.empty() && !active_;
}

void Emitter::Debug() {
    ImGui::PushID(uuid_.c_str());
    if (ImGui::TreeNode((name_ + " (" + uuid_ + ")").c_str())){
        ImGui::Text("Texture: %s", texture_.c_str());
        DebugUIWidgets::Checkbox("Billboard", &billboard_);
        DebugUIWidgets::DragFloat3("Position", &position_.x, 0.1f);
        DebugUIWidgets::DragFloat3("Rotation", &rotation_.x, 0.01f);
        DebugUIWidgets::DragFloat3("Rotation Velocity", &rotationVelocity_.x, 0.01f);
        DebugUIWidgets::DragInt("Spawn Count", reinterpret_cast<int*>(&spawnCount_), 1.f, 1, 1000);
        DebugUIWidgets::DragFloat("Frequency", &frequency_, 0.01f, 0.f, 100.f);
        DebugUIWidgets::DragFloat("Duration", &duration_, 0.01f, 0.f, 100.f);
        DebugUIWidgets::DragFloat3("Size", &size_.x, 0.01f, 0.f, 100.f);
        DebugUIWidgets::ColorEdit4("Color", &color_.x);

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

Emitter& Emitter::SetBillboard(bool _billboard) {
    billboard_ = _billboard;
    return *this;
}

Emitter& Emitter::SetPrimitive(PrimitiveType _type) {
    primitive_ = _type;
    return *this;
}

Emitter& Emitter::SetRotation(const Vector3& _rotation) {
    rotation_ = _rotation;
    return *this;
}

Emitter& Emitter::SetRotationVelocity(const Vector3& _rotationVelocity) {
    rotationVelocity_ = _rotationVelocity;
    return *this;
}

Emitter& Emitter::SetParticleLifetime(const float& _lifetime) {
    particleLifetime_ = _lifetime;
    return *this;
}

Emitter& Emitter::SetColorKeys(std::vector<GradientKey<Vector4>> _keys) {
    SortGradient(_keys);
    colorKeys_ = std::move(_keys);
    return *this;
}

Emitter& Emitter::SetSizeKeys(std::vector<GradientKey<Vector3>> _keys) {
    SortGradient(_keys);
    sizeKeys_ = std::move(_keys);
    return *this;
}

Emitter& Emitter::SetUpdateFunction(const std::function<void(float, const Vector3&, Vector3&, Vector3&, Vector4&)>& _func) {
    updateFunc_ = _func;
    return *this;
}

Emitter& Emitter::SetSpawnFunction(const std::function<void(const Vector3&, Vector3&, Vector3&)>& _func) {
    spawnFunc_ = _func;
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
    if (frequency_ <= 0.01f) {
        active_ = false;
        return;
    }

    elapsedTime_ += Time::GetDeltaTime();
    if (elapsedTime_ >= duration_) {
        active_ = false;
        return;
    }

    if (frequency_ <= timer_) {
        timer_ = 0.f;
        Spawn(spawnCount_);
        return;
    }

    timer_ += Time::GetDeltaTime();
}

void Emitter::Spawn(const uint16_t& _count) {
    for (uint16_t i = 0; i < _count; ++i) {
        Vector3 spawnPos = position_;
        Vector3 spawnVel = velocity_;
        if (spawnFunc_) {
            spawnFunc_(position_, spawnPos, spawnVel);
        }

        std::unique_ptr<Particle> particle = std::make_unique<Particle>();
        particle->SetOrigin(position_)
            .SetPosition(spawnPos)
            .SetScale(size_)
            .SetColor(color_)
            .SetVelocity(spawnVel)
            .SetRotation(rotation_)
            .SetRotationVelocity(rotationVelocity_)
            .SetColorKeys(colorKeys_)
            .SetSizeKeys(sizeKeys_)
            .SetUpdateFunction(updateFunc_)
            .Initialize(particleLifetime_);
        particles_.emplace_back(std::move(particle));
    }
}

void Emitter::RegisterGpu() {
    const auto& ac = Singleton<CameraController>::GetInstance()->GetActive();

    Matrix4x4 rotation;
    if (billboard_) {
        rotation = BackToFront * ac->GetMatrix();
        rotation.matrix[3][0] = 0.f;
        rotation.matrix[3][1] = 0.f;
        rotation.matrix[3][2] = 0.f;
    } else {
        rotation = MathUtils::Matrix::MakeIdentity();
    }

    actives_ = 0;

    std::erase_if(particles_, [&](const auto& _p) { return _p->IsDead(); });

    if (particles_.empty()) return;

    for (auto& particle : particles_) {
        particle->Update();

        const Vector3 rot = particle->GetRotation();
        const Matrix4x4 particleRot = MathUtils::Matrix::MakeRotateX(rot.x)
                                    * MathUtils::Matrix::MakeRotateY(rot.y)
                                    * MathUtils::Matrix::MakeRotateZ(rot.z);

        mapped_[actives_].world = MathUtils::Matrix::MakeAffineMatrix(
            MathUtils::Matrix::MakeScaleMatrix(particle->GetScale()),
            particleRot * rotation,
            MathUtils::Matrix::MakeTranslateMatrix(particle->GetPosition())
        );
        mapped_[actives_].wvp = mapped_[actives_].world * ac->GetViewProjection();
        mapped_[actives_].color = particle->GetColor();
        ++actives_;
    }
}
