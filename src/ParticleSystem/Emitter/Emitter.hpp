#ifndef Emitter_HPP_
#define Emitter_HPP_
#include <memory>
#include <optional>
#include <span>

#include "Math/MathUtils.hpp"
#include "Math/Vector3.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/Resource/DX12Resource.hpp"
#include "src/Mesh/Mesh.hpp"
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

    struct ForGpu {
        Matrix4x4 wvp;
        Matrix4x4 world;
        Vector4 color;
    };

    const uint16_t MAX = 1024;
    const Matrix4x4 BackToFront = MathUtils::Matrix::MakeRotateY(MathUtils::F_PI);

    std::optional<const std::reference_wrapper<DirectXAdapter>> adapter_;
    std::optional<const std::reference_wrapper<SRVManager>> srv_;

    Vector3 position_{};
    std::vector<std::unique_ptr<Particle>> particles_;

    uint16_t actives_ = 0;

    std::unique_ptr<DX12Resource> resource_;
    std::span<ForGpu> mapped_;

    uint32_t index_ = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE handle_;

    std::unique_ptr<Mesh> mesh_;
    //std::unique_ptr<ComputePipeline> pipeline_;

public:
    Emitter(DirectXAdapter& _adapter, SRVManager& _srv);
    void Initialize(std::unique_ptr<Mesh> _mesh);
    void Update();
    void Draw();

    void Spawn(const uint16_t& _count = 1);

private:
    void RegisterGpu();
}; // class Emitter

#endif // Emitter_HPP_
