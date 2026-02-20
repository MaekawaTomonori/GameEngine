#ifndef Emitter_HPP_
#define Emitter_HPP_
#include <memory>
#include <span>

#include "Math/MathUtils.hpp"
#include "Math/Vector3.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/Resource/DX12Resource.hpp"
#include "src/Mesh/Data/MeshData.hpp"
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

    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;

    bool active_ = false;

    Vector3 position_{};
    float duration_ = 1.f;
    float frequency_ = 0.f;
    uint16_t spawnCount_ = 1;
    Vector3 size_{ 1.f, 1.f, 1.f };
    Vector3 velocity_ {};
    Vector4 color_{ 1.f, 1.f, 1.f, 1.f };
    float timer_ = 0.f;

    std::function<void(const Vector3&, Vector3&, Vector3&)> spawnFunc_;
    std::function<void(float, const Vector3&, Vector3&, Vector3&, Vector4&)> updateFunc_;

    std::vector<std::unique_ptr<Particle>> particles_;

    uint16_t actives_ = 0;

    std::string name_ = "Emitter";
    std::string uuid_;
    MeshData data_;

    std::string texture_ = "white_x16.png";

    std::unique_ptr<DX12Resource> resource_;
    std::span<ForGpu> mapped_;

    uint32_t index_ = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE handle_;

    std::unique_ptr<DX12Resource> vr_;
    D3D12_VERTEX_BUFFER_VIEW vbv_{};
    std::span<Vertex> vd_;

    std::unique_ptr<DX12Resource> ir_;
    D3D12_INDEX_BUFFER_VIEW ibv_{};
    std::span<uint32_t> id_;

public:
    Emitter(DirectXAdapter* _adapter, SRVManager* _srv);
    void Initialize(const MeshData& _mesh);
    void Update();
    void Draw();
    void Emit();

    void Debug();

    /** @brief パーティクルをクリアしタイマーをリセットして再利用可能状態にする（GPUリソースは保持） **/
    void Reset();

    /** @brief 全パーティクルが死亡し、追加スポーンもない場合trueを返す **/
    bool IsFinished() const;

    Emitter& Enable(bool _active = true);

    Emitter& SetPosition(const Vector3& _position);

    Emitter& SetFrequency(const float& _frequency);

    Emitter& SetDuration(const float& _duration);

    Emitter& SetSpawnCount(const uint16_t& _count);

    Emitter& SetColor(const Vector4& _color);

    Emitter& SetTexture(const std::string& _texture);

    Emitter& SetSize(const float& _size);

    Emitter& SetSize(const Vector3& _size);

    Emitter& SetVelocity(const Vector3& _velocity);

    /** @brief パーティクル更新関数を設定
     ** @param _func <float(time)>, <const Vector3&(origin)>, <Vector3&(position)>, <Vector3&(velocity)>, <Vector4&(color)>
     **/
    Emitter& SetUpdateFunction(const std::function<void(float, const Vector3&, Vector3&, Vector3&, Vector4&)>& _func);

    /** @brief パーティクルスポーン関数を設定（スポーン時の初期位置・速度をカスタマイズ）
     ** @param _func <const Vector3&(emitterCenter)>, <Vector3&(outPos)>, <Vector3&(outVel)>
     **/
    Emitter& SetSpawnFunction(const std::function<void(const Vector3&, Vector3&, Vector3&)>& _func);

private:
    void FrequencyUpdate();
    void Spawn(const uint16_t& _count);
    void RegisterGpu();
}; // class Emitter

#endif // Emitter_HPP_
