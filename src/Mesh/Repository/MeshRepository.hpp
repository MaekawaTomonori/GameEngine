#ifndef MESH_REPOSITORY_HPP_
#define MESH_REPOSITORY_HPP_
#include <memory>
#include <string>
#include <unordered_map>

#include "src/Mesh/Mesh.hpp"
#include "src/DirectX/DirectXAdapter.hpp"

class MeshRepository {
    DirectXAdapter* adapter_ = nullptr;

    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes_;

public:
    void Initialize(DirectXAdapter* _adapter);

    Mesh* Add(const std::string &_name);

    void Draw(const std::string& _name);
}; // class MeshRepository

#endif // MeshLoader_HPP_
