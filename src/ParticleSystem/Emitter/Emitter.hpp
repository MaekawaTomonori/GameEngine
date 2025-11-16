#ifndef Emitter_HPP_
#define Emitter_HPP_
#include <memory>
#include <optional>

#include "Math/Vector3.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"
#include "src/ParticleSystem/Particle/Particle.hpp"
//#include "src/DirectX/Compute/ComputePipeline.hpp"

class Emitter {
    //struct ParticleForGpu {
    //    Vector3 position;
    //    Vector3 scale;
    //    float lifetime;
    //    Vector3 velocity;
    //    float timer;
    //    Vector4 color;
    //};

    const uint16_t MAX = 1024;

    std::optional<std::reference_wrapper<DirectXAdapter>> adapter_;

    Vector3 position_{};
    std::vector<std::unique_ptr<Particle>> particles_;

    uint16_t actives_ = 10;

    std::unique_ptr<DX12Resource> resource_;
    //std::unique_ptr<ComputePipeline> pipeline_;

public:
    void Initialize(std::reference_wrapper<DirectXAdapter> _adapter);
    void Update();
    void Draw();

    void Spawn();

private:

}; // class Emitter

#endif // Emitter_HPP_
