#include "ResourceRepository.hpp"

ModelRepository* ResourceRepository::GetModelRepository() const {
    return model_.get();
}

MeshRepository* ResourceRepository::GetMeshRepository() const {
    return mesh_.get();
}
