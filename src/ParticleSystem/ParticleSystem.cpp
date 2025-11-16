#include "ParticleSystem.hpp"

#include <stdexcept>

#include "Log.hpp"
#include "Utils.hpp"

void ParticleSystem::Initialize() { }

void ParticleSystem::Update() { }

void ParticleSystem::Draw() { }

ParticleSystem::GroupEditor ParticleSystem::Register(const std::string& _name, Vector3 _position) {
    if (groups_.contains(_name)) {
        Log::Send(Log::Level::WARNING, "Group already exists.");
        return { groups_.at(_name).get() };
    }

    groups_.emplace(_name, std::make_unique<Group>(_position));

    return {groups_.at(_name).get()};
}

ParticleSystem::GroupEditor ParticleSystem::Edit(const std::string& _name) const {
    if (groups_.contains(_name)) {
        return {groups_.at(_name).get()};
    }

    Log::Send(Log::Level::WARNING, "Group not found.");
    Utils::Alert("Group not found.");
    throw std::runtime_error("Group not found.");
}
