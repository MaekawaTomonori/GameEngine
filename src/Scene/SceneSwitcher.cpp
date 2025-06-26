#include "SceneSwitcher.hpp"

void SceneSwitcher::Update() {
    // Scene Changer
    //if (changer_->InProgress()){
    //    changer_->Update();
    //    return;
    //}

    if (next_){
        if (scene_){
            scene_->Finalize();
            scene_.reset();
            scene_ = nullptr;
        }
        scene_ = std::move(next_);

        next_.reset();
        next_ = nullptr;
        scene_->SetSwitcher(this);
        scene_->Initialize();
        scene_->Update();

        //changer_->Awake(scene_->GetEntryEffect(), ISceneEffect::State::In);
    }

    if (!scene_)return;

    // ChangeProcess End
    //if (changer_->InProgress())return;

    //if (!scene_->InProgress()){
        scene_->Awake();
    //}

    scene_->Update();
}

void SceneSwitcher::Draw() {
    if (scene_){ 
        scene_->Draw();
    }
}

void SceneSwitcher::SetFactory(std::unique_ptr<AbstractSceneFactory> _factory) {
    factory_ = std::move(_factory);
}

void SceneSwitcher::Change(const std::string &_name) {
    if (!factory_) return;

    // Already Registered
    if (next_) return;

    next_ = factory_->Create(_name);

    // out fade
    // Already Destroyed
    if (!scene_) return;

    // Fade
}
