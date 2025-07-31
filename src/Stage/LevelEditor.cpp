#include "LevelEditor.hpp"

#include "Log.hpp"
#include "imgui_internal.h"

LevelEditor::LevelEditor(DebugUI* _debug) :debug_(_debug), loader_(std::make_unique<StageLoader>()), repository_(std::make_unique<StageRepository>()){
    loader_->Initialize(repository_.get());
}

void LevelEditor::Initialize(const std::string& _name) {
    objects_.clear();

    current_ = _name;

    Spawn();

    tick_ = 0;
}

void LevelEditor::Update() {
    Debug();

    if (reload_) {
        reload_ = false;
        Spawn();
    }

    if (objects_.empty()){
        return;
    }
    for (auto& object : objects_){
        object.model->Update();
    }
    tick_++;
}

void LevelEditor::Draw() const {
    if (objects_.empty()){
        return;
    }
    for (const auto& object : objects_){
        object.model->Draw();
    }
}

void LevelEditor::Spawn() {
    if (!loader_->Load(current_)){
        Log::Send(Log::Level::ERR, "Failed to load level: " + current_);
        Utils::Alert("Failed to load level: " + current_);
        return;
    }
    LevelData* data = repository_->Get(current_);

    std::vector<StageObject> newObjects;
    for (auto& object : data->objects) {
        StageObject stageObject;
        stageObject.model = std::make_unique<Model>();
        stageObject.model->Initialize(object.file);
        stageObject.model->SetTranslate(object.translate);
        stageObject.model->SetRotate(object.rotate);
        stageObject.model->SetScale(object.scale);
        newObjects.push_back(std::move(stageObject));
    }

    objects_ = std::move(newObjects);
}

void LevelEditor::Debug() {
    debug_->RegisterCommand("Level", [&]() {
        ImGui::Begin("Level Editor");
        if (ImGui::Button("Reload")) {
            reload_ = true;
        }
        ImGui::End();
    });
}
