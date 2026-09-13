#ifndef PerformanceTestScene_HPP_
#define PerformanceTestScene_HPP_
#include <memory>
#include <vector>

#include "IScene.hpp"
#include "Model.hpp"

/** @brief モデル数・パーティクル発生頻度を段階的に増減させ、
 ** 現在のエンジン性能を計測するためのデバッグ用シーン。
 ** SceneSwitcherデバッグパネルから "perf" として切り替えて使う。
 */
class PerformanceTestScene final : public IScene {
    struct Entry {
        std::unique_ptr<Model> model;
        Vector3 position;
        Vector3 rotation;
    };
    std::vector<Entry> models_;

    int targetModelCount_ = 0;
    float modelSpawnRatePerSecond_ = 10.f;
    float modelSpawnAccumulator_ = 0.f;

    float particleEmitRatePerSecond_ = 5.f;
    float particleEmitAccumulator_ = 0.f;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;

private:
    void UpdateModelSpawning(float _deltaTime);
    void UpdateParticleEmission(float _deltaTime);
    void SpawnModel();
    void DespawnModel();
}; // class PerformanceTestScene

#endif // PerformanceTestScene_HPP_
