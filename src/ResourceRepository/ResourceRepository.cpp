#include "ResourceRepository.hpp"

void ResourceRepository::Initialize() {
    model_ = std::make_unique<ModelRepository>();
    mesh_ = std::make_unique<MeshRepository>();
}

GESTD::ReferencePtr<ModelRepository> ResourceRepository::GetModelRepository() const {
    return GESTD::ReferencePtr<ModelRepository>(model_);
}

GESTD::ReferencePtr<MeshRepository> ResourceRepository::GetMeshRepository() const {
    return GESTD::ReferencePtr<MeshRepository>(mesh_);
}
