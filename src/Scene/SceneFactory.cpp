#include "SceneFactory.hpp"

#include <ranges>
#include <stdexcept>

#include "IScene.hpp"
#include "Log.hpp"
#include "Utils.hpp"

void SceneFactory::Register(const std::string& _name, const CreateFunc& _creator) {
    creators_[_name] = _creator;
    Log::Send(Log::Level::INFO, "SceneRegistered : " + _name);
}

std::unique_ptr<IScene> SceneFactory::Create(const std::string& _name) {
    if (creators_.contains(_name)) {
        return creators_[_name]();
    }

    Utils::Alert("Scene not found: " + _name);
    throw std::runtime_error("Scene not found: " + _name);
}

std::vector<std::string> SceneFactory::GetRegisteredScenes() const {
    std::vector<std::string> scenes;
    scenes.reserve(creators_.size());
    for (const auto& name : creators_ | std::views::keys) {
        scenes.push_back(name);
    }
    return scenes;
}