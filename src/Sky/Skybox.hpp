#ifndef Skybox_HPP_
#define Skybox_HPP_
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "src/DirectX/Heap/SRVManager.h"


class Skybox {
    struct VertexData{
        Vector4 position;
        Vector2 uv;
        Vector3 normal;
    };

    std::string texture_;

    std::unique_ptr<DX12Resource> vr_;
    D3D12_VERTEX_BUFFER_VIEW vbv_ {};

public:
    void Initialize(const std::string& _texture);

private:
    void CreateVertex();
}; // class Skybox

#endif // Skybox_HPP_
