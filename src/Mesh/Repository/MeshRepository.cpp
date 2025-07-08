#include "MeshRepository.hpp"

#include "Log.hpp"

void MeshRepository::Initialize(DirectXAdapter *_adapter) {
    adapter_ = _adapter;
}

Mesh* MeshRepository::Add(const std::string &_name) {
    if (meshes_.contains(_name))return meshes_[_name].get();

    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
    mesh->Initialize(adapter_, _name);

    meshes_[_name] = std::move(mesh);

    Log::Send(Log::Level::INFO, "Mesh Loaded: " + _name);
    return meshes_[_name].get();
}

void MeshRepository::Draw(const std::string &_name) {
    if (!meshes_.contains(_name)){
        Log::Send(Log::Level::ERR, "Mesh not found: " + _name);
        return;
    }
    meshes_[_name]->Draw();
}
