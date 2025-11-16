#include "Emitter.hpp"

#include "Log.hpp"
#include "Utils.hpp"

void Emitter::Initialize(std::reference_wrapper<DirectXAdapter> _adapter) {
    adapter_ = _adapter;

    if (!adapter_.has_value()) {
        Log::Send(Log::Level::ERR, "Emitter failed to initialize: DirectXAdapter not available");
        Utils::Alert("Emitter failed to initialize: DirectXAdapter not available");
        throw std::runtime_error("Emitter failed to initialize: DirectXAdapter not available");
    }

    const auto& adapter = adapter_.value().get();

    resource_ = adapter.CreateBufferResource(sizeof(ParticleForGpu) * MAX);
}

void Emitter::Update() { }
void Emitter::Draw() {
    
}

void Emitter::Spawn() { }
