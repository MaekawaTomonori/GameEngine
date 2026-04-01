#include "ResourceRepository.hpp"

void ResourceRepository::Initialize() {
    model_ = std::make_unique<ModelRepository>();
    mesh_ = std::make_unique<MeshRepository>();
}

GESTD::WeakPtr<ModelRepository> ResourceRepository::GetModelRepository() const {
    return GESTD::WeakPtr<ModelRepository>(model_);
}

GESTD::WeakPtr<MeshRepository> ResourceRepository::GetMeshRepository() const {
    return GESTD::WeakPtr<MeshRepository>(mesh_);
}
