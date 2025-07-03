#include "MeshManager.hpp"

#include "Log.hpp"

void MeshManager::Initialize(DirectXAdapter *_adapter) {
    adapter_ = _adapter;
}

Mesh* MeshManager::Load(const std::string &_name) {
    if (meshes_.contains(_name))return meshes_[_name].get();

    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
    mesh->Initialize(adapter_, ASSETS_FOLDER, _name);

    meshes_[_name] = std::move(mesh);

    Log::Send(Log::Level::INFO, "Mesh Loaded: " + _name);
    return meshes_[_name].get();
}

void MeshManager::Draw(const std::string &_name) {
    if (!meshes_.contains(_name)){
        Log::Send(Log::Level::ERR, "Mesh not found: " + _name);
        return;
    }
    meshes_[_name]->Draw();
}
