#ifndef Mesh_HPP_
#define Mesh_HPP_
#include <d3d12.h>
#include <string>
#include <wrl/client.h>

#include "Data/MeshData.hpp"
#include "src/DirectX/DirectXAdapter.hpp"

class Mesh {
    struct Material {
        Vector4 color;
        uint32_t lighting;
        float shininess;
    };

    DirectXAdapter* adapter_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    std::string name_;

    MeshData data_;

    // vertex resource
    Microsoft::WRL::ComPtr<ID3D12Resource> vr_;
    // vertex buffer view
    D3D12_VERTEX_BUFFER_VIEW vbv_ {};
    // vertex data
    Vertex* vd_ = nullptr;

    D3D12_VERTEX_BUFFER_VIEW skinning_{};

    // index resource
    Microsoft::WRL::ComPtr<ID3D12Resource> ir_;
    // index buffer view
    D3D12_INDEX_BUFFER_VIEW ibv_ {};
    // index
    uint32_t* id_ = nullptr;

    // material resource
    Microsoft::WRL::ComPtr<ID3D12Resource> mr_;
    // material
    Material* material_ = nullptr;

    // current texture
    std::string texture_;

    // lighting
	bool lighting_ = false;

public:
    void Initialize(DirectXAdapter* _adapter, const std::string &_name, const MeshData& _raw);
    void Update();
    void Draw() const;
    void Debug();

    void SetVBV(D3D12_VERTEX_BUFFER_VIEW _vbv);
}; // class Mesh

#endif // Mesh_HPP_
