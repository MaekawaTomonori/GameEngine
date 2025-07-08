#ifndef MESH_DATA_HPP_
#define MESH_DATA_HPP_
#include <string>
#include <vector>

#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"

struct Vertex {
    Vector4 position;
    Vector2 uv;
    Vector3 normal;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::string texture;
};

#endif // MeshData_HPP_
