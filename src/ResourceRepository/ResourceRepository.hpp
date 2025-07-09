#ifndef ResourceRepository_HPP_
#define ResourceRepository_HPP_
#include <memory>

#include "src/Mesh/Repository/MeshRepository.hpp"
#include "src/Model/Repository/ModelRepository.hpp"

class ResourceRepository {
    std::unique_ptr<ModelRepository> model_;
    std::unique_ptr<MeshRepository> mesh_;
public:
    ResourceRepository();
    ModelRepository* GetModelRepository() const;
    MeshRepository* GetMeshRepository() const;
}; // class ResourceRepository

#endif // ResourceRepository_HPP_
