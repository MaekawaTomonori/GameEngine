#include "PerformanceTestScene.hpp"

#include "imgui.h"
#include "src/Time/Time.hpp"

namespace {
    constexpr const char* TEST_TEMPLATE = "perf_test_particle";
    constexpr int GRID_WIDTH = 32;
    constexpr float GRID_SPACING = 2.0f;
    constexpr int MAX_SPAWN_PER_FRAME = 20;
}

void PerformanceTestScene::Initialize() {
    ParticleSystem::EmitterConfig config;
    config.texture = "white_x16.png";
    config.frequency = 0.f;
    config.duration = 0.f;
    config.spawnCount = 10;
    config.size = {0.3f, 0.3f, 0.3f};
    config.velocity = {0.f, 2.f, 0.f};
    config.color = {1.f, 1.f, 1.f, 1.f};
    config.particleLifetime = 1.5f;
    config.colorKeys = {
        GradientKey<Vector4>{0.0f, {1.f, 1.f, 1.f, 1.f}},
        GradientKey<Vector4>{1.0f, {1.f, 1.f, 1.f, 0.f}}
    };

    ParticleSystem::Template tmpl;
    tmpl.emitters.push_back(config);
    Particle()->Register(TEST_TEMPLATE, tmpl, true);
}

void PerformanceTestScene::Update() {
    const float dt = Time().GetDeltaTime();

    for (auto& entry : models_) {
        entry.model->Update();
    }

    UpdateModelSpawning(dt);
    UpdateParticleEmission(dt);
}

void PerformanceTestScene::UpdateModelSpawning(float _deltaTime) {
    if (modelSpawnRatePerSecond_ <= 0.f) return;

    const float interval = 1.f / modelSpawnRatePerSecond_;
    modelSpawnAccumulator_ += _deltaTime;

    int processedThisFrame = 0;
    while (modelSpawnAccumulator_ >= interval && processedThisFrame < MAX_SPAWN_PER_FRAME) {
        if (static_cast<int>(models_.size()) < targetModelCount_) {
            SpawnModel();
        } else if (static_cast<int>(models_.size()) > targetModelCount_) {
            DespawnModel();
        } else {
            break;
        }
        modelSpawnAccumulator_ -= interval;
        ++processedThisFrame;
    }
}

void PerformanceTestScene::UpdateParticleEmission(float _deltaTime) {
    if (particleEmitRatePerSecond_ <= 0.f) return;

    const float interval = 1.f / particleEmitRatePerSecond_;
    particleEmitAccumulator_ += _deltaTime;

    while (particleEmitAccumulator_ >= interval) {
        Particle()->Emit(TEST_TEMPLATE, {0.f, 1.f, 0.f});
        particleEmitAccumulator_ -= interval;
    }
}

void PerformanceTestScene::SpawnModel() {
    Entry entry;
    entry.model = std::make_unique<Model>();
    entry.model->Initialize("plane");

    const int index = static_cast<int>(models_.size());
    const float x = static_cast<float>(index % GRID_WIDTH) * GRID_SPACING;
    const float z = static_cast<float>(index / GRID_WIDTH) * GRID_SPACING;
    entry.position = {x, 0.f, z};
    entry.rotation = {4.5f, 0.f, 0.f};
    entry.model->SetTranslate(entry.position);
    entry.model->SetRotate(entry.rotation);
    entry.model->SetScale({0.8f, 0.8f, 1.f});

    models_.push_back(std::move(entry));
}

void PerformanceTestScene::DespawnModel() {
    if (!models_.empty()) {
        models_.pop_back();
    }
}

void PerformanceTestScene::Draw() {
    for (auto& entry : models_) {
        entry.model->Draw();
    }
}

void PerformanceTestScene::Debug() {
    ImGui::Begin("PerformanceTest");
    ImGui::Text("Active Models: %d / %d", static_cast<int>(models_.size()), targetModelCount_);
    ImGui::SliderInt("Target Model Count", &targetModelCount_, 0, 1000);
    ImGui::SliderFloat("Model Spawn Rate (per sec)", &modelSpawnRatePerSecond_, 0.f, 200.f);
    ImGui::Separator();
    ImGui::SliderFloat("Particle Emit Rate (per sec)", &particleEmitRatePerSecond_, 0.f, 60.f);
    ImGui::Separator();
    ImGui::Text("Bulk Model Edit (applies once per click)");

    static Vector3 rotationDelta = {0.f, 0.f, 0.f};
    ImGui::DragFloat3("Rotation Delta", &rotationDelta.x, 0.01f);
    if (ImGui::Button("Apply Rotation To All")) {
        for (auto& entry : models_) {
            entry.rotation += rotationDelta;
            entry.model->SetRotate(entry.rotation);
        }
    }

    static Vector3 positionDelta = {0.f, 0.f, 0.f};
    ImGui::DragFloat3("Position Delta", &positionDelta.x, 0.1f);
    if (ImGui::Button("Apply Position Offset To All")) {
        for (auto& entry : models_) {
            entry.position += positionDelta;
            entry.model->SetTranslate(entry.position);
        }
    }
    ImGui::End();
}
