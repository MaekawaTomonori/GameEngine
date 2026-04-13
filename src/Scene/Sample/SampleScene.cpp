#include "SampleScene.hpp"

void SampleScene::Initialize() {
    model_ = std::make_unique<Model>();
    model_->Initialize("animatedCube");
    model_->SetTranslate({0.f, .5f, -5.f});
    model_->SetScale({0.4f, 0.4f, 0.4f});
}

void SampleScene::Update() {
    model_->Update();
}

void SampleScene::Draw() {
    model_->Draw();
}

void SampleScene::Debug() {
}
