#ifndef MESH_REPOSITORY_HPP_
#define MESH_REPOSITORY_HPP_
#include <string>
#include <unordered_map>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/Mesh/Data/MeshData.hpp"

class MeshRepository {
    DirectXAdapter* adapter_ = nullptr;

    std::unordered_map<std::string, MeshData> data_;

public:
    void Initialize(DirectXAdapter* _adapter);

    void Add(const std::string &_name, const MeshData& _raw);
    MeshData Get(const std::string& _name);
    bool Contains(const std::string& _name) const;
}; // class MeshRepository

#endif // MeshLoader_HPP_
