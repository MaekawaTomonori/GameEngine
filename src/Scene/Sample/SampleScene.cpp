#include "SampleScene.hpp"

#include "Input.hpp"
#include "Pattern/Singleton.hpp"

void SampleScene::Initialize() {
    model_ = std::make_unique<Model>();
    model_->Initialize("animatedCube");
    model_->SetTranslate({0.f, .5f, 0.f});
    model_->SetScale({0.4f, 0.4f, 0.4f});

    plane = std::make_unique<Model>();
    plane->Initialize("plane");
    plane->SetTranslate({0.f, -0.4f, 0.f});
    plane->SetRotate({4.5f, 0.f, 0.f});
    plane->SetScale({5.f, 5.f, 1.f});

    kick_.emplace("Assets/audio/kick.wav");
}

void SampleScene::Update() {
    model_->Update();
    plane->Update();

    if (kick_ && Singleton<Input>::GetInstance()->IsTrigger(DIK_SPACE)) {
        kick_->Play();
    }

}

void SampleScene::Draw() {
    model_->Draw();
    plane->Draw();
}

void SampleScene::Debug() {
}
