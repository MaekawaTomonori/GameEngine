#include "StageLoader.hpp"

#include <fstream>

#include "Log.hpp"
#include "Utils.hpp"
#include "vendor/json/json.hpp"

void StageLoader::Initialize(StageRepository* _repository) {
    repository_ = _repository;
}

bool StageLoader::Load(const std::string& _path) const {
    std::string fullPath = DIR + _path + ".json";

    Log::Send(Log::Level::INFO, "StageLoader::Load " + fullPath);

    std::ifstream file;
    file.open(fullPath);
    if (file.fail()) {
        Log::Send(Log::Level::ERR, "Failed to open stage file: " + fullPath);
        Utils::Alert("Failed to open stage file: " + fullPath);
        return false;
    }

    nlohmann::json deserialized;
    try{
        file >> deserialized;
    } catch (const nlohmann::json::parse_error& e){
        Log::Send(Log::Level::ERR, "JSON parse error: " + std::string(e.what()));
        Utils::Alert("JSON parse error: " + std::string(e.what()));
        return false;
    }

    std::string stage;

    if (deserialized.is_object() && deserialized.contains("name") && deserialized["name"].is_string()){
        stage = deserialized["name"].get<std::string>();
        Log::Send(Log::Level::INFO, "Stage name: " + stage);
    } else{
        Log::Send(Log::Level::ERR, "Invalid stage file format: " + fullPath);
        Utils::Alert("Invalid stage file format: " + fullPath);
        return false;
    }

    if (stage.empty()){
        Log::Send(Log::Level::ERR, "Stage name is empty in file: " + fullPath);
        Utils::Alert("Stage name is empty in file: " + fullPath);
        return false;
    }

    if (!Utils::EqualsIgnoreCase(stage, "scene")) {
        Log::Send(Log::Level::ERR, "Invalid stage name: " + stage + ". Expected 'scene'.");
        Utils::Alert("Invalid stage name: " + stage + ". Expected 'scene'.");
        return false;
    }

    repository_->Add(_path, Recursive(deserialized));

    return true;
}

std::unique_ptr<LevelData> StageLoader::Recursive(nlohmann::json _base) {
    std::unique_ptr<LevelData> data = std::make_unique<LevelData>();

    for (nlohmann::json object : _base["objects"]){
        if (!object.contains("type")){
            Log::Send(Log::Level::ERR, "Object type is missing in stage file: " + _base.at("name").get<std::string>());
            Utils::Alert("Object type is missing in stage file: " + _base.at("name").get<std::string>());
        }

        std::string type = object["type"].get<std::string>();

        data->objects.emplace_back(LevelData::ObjectData{});
        LevelData::ObjectData& objectData = data->objects.back();

        if (object.contains("name")){
            objectData.name = object["name"].get<std::string>();
        } else{
            Log::Send(Log::Level::ERR, "Mesh object missing 'name' in stage file: " + _base.at("name").get<std::string>());
            Utils::Alert("Mesh object missing 'name' in stage file: " + _base.at("name").get<std::string>());
        }

        if (!object.contains("transform")){
            Log::Send(Log::Level::ERR, "Mesh object missing 'transform' in stage file: " + _base.at("name").get<std::string>());
            Utils::Alert("Mesh object missing 'transform' in stage file: " + _base.at("name").get<std::string>());
        }

        nlohmann::json& transform = object["transform"];

        objectData.translate.x = transform["translation"][0].get<float>();
        objectData.translate.y = transform["translation"][2].get<float>();
        objectData.translate.z = transform["translation"][1].get<float>();

        objectData.rotate.x = -transform["rotation"][0].get<float>();
        objectData.rotate.y = -transform["rotation"][2].get<float>();
        objectData.rotate.z = -transform["rotation"][1].get<float>();

        objectData.scale.x = transform["scale"][0].get<float>();
        objectData.scale.y = transform["scale"][2].get<float>();
        objectData.scale.z = transform["scale"][1].get<float>();

        if (object.contains("children")){
            Recursive(object["children"]);
        }
    }

    return std::move(data);
}
