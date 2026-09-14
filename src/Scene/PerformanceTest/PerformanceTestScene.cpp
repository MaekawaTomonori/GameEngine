#include "PerformanceTestScene.hpp"

#include "imgui.h"
#include "src/Time/Time.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Controller/CameraController.hpp"

namespace {
    constexpr const char* TEST_TEMPLATE = "perf_test_particle";
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

    floor_ = std::make_unique<Model>();
    floor_->Initialize("plane");
    floor_->SetTexture("white_x16.png");
    floor_->SetColor({0.5f, 0.5f, 0.5f, 1.0f});
    floor_->SetTranslate({0.f, 0.f, 0.f});
    floor_->SetRotate({-1.5707963f, 0.f, 0.f});
    floor_->SetScale({40.f, 40.f, 1.f});
}

void PerformanceTestScene::Update() {
    const float dt = Time().GetDeltaTime();

    floor_->Update();

    for (auto& model : models_) {
        model->Update();
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
    auto model = std::make_unique<Model>();
    model->Initialize("cube");

    const int index = static_cast<int>(models_.size());
    const float x = static_cast<float>(index % gridWidth_) * gridSpacing_;
    const float z = static_cast<float>(index / gridWidth_) * gridSpacing_;
    model->SetTranslate({x, 0.f, z});
    model->SetScale({0.8f, 0.8f, 0.8f});

    models_.push_back(std::move(model));
}

void PerformanceTestScene::DespawnModel() {
    if (!models_.empty()) {
        models_.pop_back();
    }
}

void PerformanceTestScene::Draw() {
    floor_->Draw();

    for (auto& model : models_) {
        model->Draw();
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
    ImGui::Text("Grid Layout");
    ImGui::SliderInt("Grid Width", &gridWidth_, 1, 64);
    ImGui::DragFloat("Grid Spacing", &gridSpacing_, 0.1f, 0.5f, 10.f);

    ImGui::Separator();
    ImGui::Text("Camera Adjust (Active Camera)");
    auto camera = Singleton<CameraController>::GetInstance()->GetActive();
    if (camera) {
        ImGui::DragFloat3("Camera Position", &camera->transform_.translate.x, 0.1f);
        if (std::holds_alternative<Vector3>(camera->transform_.rotate)) {
            ImGui::DragFloat3("Camera Rotation", &std::get<Vector3>(camera->transform_.rotate).x, 0.01f);
        }
    }
    ImGui::End();
}
