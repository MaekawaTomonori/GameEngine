#include "ResourceRepository.hpp"

void ResourceRepository::Initialize() {
    model_ = std::make_unique<ModelRepository>();
    mesh_ = std::make_unique<MeshRepository>();
}

ModelRepository* ResourceRepository::GetModelRepository() const {
    return model_.get();
}

MeshRepository* ResourceRepository::GetMeshRepository() const {
    return mesh_.get();
}
