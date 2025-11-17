#include "Emitter.hpp"

#include "Log.hpp"
#include "Utils.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Controller/CameraController.hpp"

Emitter::Emitter(DirectXAdapter& _adapter, SRVManager& _srv) :adapter_(_adapter), srv_(_srv), handle_() { }

void Emitter::Initialize(std::unique_ptr<Mesh> _mesh) {
    if (!adapter_.has_value()) {
        Utils::Alert("Emitter failed to initialize: DirectXAdapter not available");
        throw std::runtime_error("Emitter failed to initialize: DirectXAdapter not available");
    }

    if (!srv_.has_value()) {
        Utils::Alert("Emitter failed to initialize: SRVManager not available");
        throw std::runtime_error("Emitter failed to initialize: SRVManager not available");
    }

    auto& adapter = adapter_->get();
    auto& srv = srv_->get();

    resource_ = adapter.CreateBufferResource(sizeof(ForGpu) * MAX);
    resource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));

    index_ = srv.Allocate();
    handle_ = srv.GetGPUHandle(index_);
    srv.CreateSRVforStructuredBuffer(index_, resource_->Get(), MAX, sizeof(ForGpu));

    if (!_mesh) {
        Utils::Alert("Emitter initialized without mesh");
        return;
    }

    mesh_ = std::move(_mesh);

}

void Emitter::Update() {
    RegisterGpu();
}

void Emitter::Draw() {
    if (!adapter_.has_value())return;
    if (actives_ <= 0)return;

    const auto command = adapter_->get().GetCommandList();

    command->SetGraphicsRootDescriptorTable(1, handle_);
    mesh_->Draw(actives_);
}

void Emitter::Spawn(const uint16_t& _count) {
    (void)_count;
}

void Emitter::RegisterGpu() {
    const auto& ac = Singleton<CameraController>::GetInstance()->GetActive();
    Matrix4x4 billboard = BackToFront * ac->GetMatrix();
    billboard.matrix[3][0] = 0.f;
    billboard.matrix[3][1] = 0.f;
    billboard.matrix[3][2] = 0.f;
    actives_ = 0;

    std::erase_if(particles_, [&](const auto& _p){ return _p->IsDead(); });
    for (auto& particle : particles_) {
        particle->Update();
        mapped_[actives_].world = MathUtils::Matrix::MakeAffineMatrix(
            MathUtils::Matrix::MakeScaleMatrix(particle->GetScale()),
            billboard,
            MathUtils::Matrix::MakeTranslateMatrix(position_ + particle->GetPosition())
        );
        mapped_[actives_].wvp = mapped_[actives_].world * ac->GetViewProjection();
        mapped_[actives_].color = particle->GetColor();
        ++actives_;
    }
}
