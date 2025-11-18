#include "ParticleSystem.hpp"

#include <ranges>
#include <stdexcept>

#include "Log.hpp"
#include "Utils.hpp"

ParticleSystem::ParticleSystem(DirectXAdapter& _adapter, SRVManager& _srv) :adapter_(_adapter), srv_(_srv) {
}

ParticleSystem::GroupEditor::GroupEditor(Group* _group) : group_(_group) {
}

ParticleSystem::GroupEditor& ParticleSystem::GroupEditor::AddEmitter() {
    //std::unique_ptr<Emitter> emitter = std::make_unique<Emitter>(adapter_, );
    //group_->emitters.push_back()
    return *this;
}

void ParticleSystem::Initialize() {

}

void ParticleSystem::Update() {
    if (groups_.empty())return;

    for (const auto& group : groups_ | std::views::values) {
        for (const auto& emitter : group->emitters) {
            emitter->Update();
        }
    }
}

void ParticleSystem::Draw(const std::reference_wrapper<Renderer> _renderer) {
    if (groups_.empty())return;

    _renderer.get().Register([&] {
        for (auto& group : groups_ | std::views::values) {
            for (auto& emitter : group->emitters) {
                emitter->Draw();
            }
        }
        }, true);
}

ParticleSystem::GroupEditor ParticleSystem::Register(const std::string& _name, Vector3 _position) {
    if (groups_.contains(_name)) {
        Log::Send(Log::Level::WARNING, "Group already exists.");
        return { groups_.at(_name).get() };
    }

    groups_.emplace(_name, std::make_unique<Group>(_position));

    return { groups_.at(_name).get() };
}

ParticleSystem::GroupEditor ParticleSystem::Edit(const std::string& _name) const {
    if (groups_.contains(_name)) {
        return { groups_.at(_name).get() };
    }

    Log::Send(Log::Level::WARNING, "Group not found.");
    Utils::Alert("Group not found.");
    throw std::runtime_error("Group not found.");
}

void ParticleSystem::Delete(const std::string& _name) {
    if (!groups_.contains(_name)) return;

    groups_.erase(_name);
}
