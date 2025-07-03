#ifndef MeshLoader_HPP_
#define MeshLoader_HPP_
#include <memory>
#include <string>
#include <unordered_map>

#include "src/Mesh/Mesh.hpp"
#include "src/DirectX/DirectXAdapter.hpp"

class MeshManager {
    DirectXAdapter* adapter_ = nullptr;

    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes_;

    const std::string ASSETS_FOLDER = "Assets/Resources/";
public:
    void Initialize(DirectXAdapter* _adapter);

    Mesh* Load(const std::string &_name);

    void Draw(const std::string& _name);
}; // class MeshManager

#endif // MeshLoader_HPP_
