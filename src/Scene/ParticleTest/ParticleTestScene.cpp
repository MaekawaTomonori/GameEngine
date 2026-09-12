#include "ParticleTestScene.hpp"

#include "Input.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Time/Time.hpp"

namespace {
    constexpr const char* TEST_TEMPLATE = "particle_test_burst";
}

void ParticleTestScene::Initialize() {
    plane_ = std::make_unique<Model>();
    plane_->Initialize("plane");
    plane_->SetTranslate({0.f, -0.4f, 0.f});
    plane_->SetRotate({4.5f, 0.f, 0.f});
    plane_->SetScale({5.f, 5.f, 1.f});

    ParticleSystem::EmitterConfig config;
    config.texture = "white_x16.png";
    config.frequency = 0.f;
    config.duration = 0.f;
    config.spawnCount = 30;
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

void ParticleTestScene::Update() {
    plane_->Update();

    timer_ += Time::GetDeltaTime();
    if (timer_ >= 1.0f) {
        timer_ = 0.f;
        Particle()->Emit(TEST_TEMPLATE, {0.f, 1.f, 0.f});
    }

    if (Singleton<Input>::GetInstance()->IsTrigger(DIK_SPACE)) {
        Particle()->Emit(TEST_TEMPLATE, {0.f, 1.f, 0.f});
    }
}

void ParticleTestScene::Draw() {
    plane_->Draw();
}

void ParticleTestScene::Debug() {
}
