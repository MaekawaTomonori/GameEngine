#ifndef Mesh_HPP_
#define Mesh_HPP_
#include <d3d12.h>
#include <string>
#include <wrl/client.h>
#include <memory>

#include "Data/MeshData.hpp"
#include "Math/Matrix.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

class Mesh {
    struct Material {
        Vector4 color;           // 16 bytes (aligned)
        uint32_t lighting;       // 4 bytes
        float shininess;         // 4 bytes  
        float coefficient;       // 4 bytes
        float pad1;              // 4 bytes (padding to 16-byte boundary)
        Vector2 tilingMul;       // 8 bytes
        Vector2 pad2;            // 8 bytes (padding to 16-byte boundary)
        Matrix4x4 uvTransform;   // 64 bytes (aligned)
    };

    DirectXAdapter* adapter_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    std::string name_;

    MeshData data_;

    // vertex resource
    std::unique_ptr<DX12Resource> vr_;
    // vertex buffer view
    D3D12_VERTEX_BUFFER_VIEW vbv_ {};
    // vertex data
    Vertex* vd_ = nullptr;

    std::vector<D3D12_VERTEX_BUFFER_VIEW> vbvs_;

    // index resource
    std::unique_ptr<DX12Resource> ir_;
    // index buffer view
    D3D12_INDEX_BUFFER_VIEW ibv_ {};
    // index
    uint32_t* id_ = nullptr;

    // material resource
    std::unique_ptr<DX12Resource> mr_;
    // material
    Material* material_ = nullptr;

    // current texture
    std::string texture_;

    // lighting
	bool lighting_ = false;

    // tiling aspect ratio lock
    bool aspectRatioLocked_ = false;
    float aspectRatio_ = 1.0f;

public:
    void Initialize(DirectXAdapter* _adapter, const std::string &_name, const MeshData& _raw);
    void Update();
    void Draw() const;
    void Debug();

    void SetVBV(D3D12_VERTEX_BUFFER_VIEW _vbv);

    MeshData GetData() const;

    // Texture management
    void SetTexture(const std::string& _texturePath);
}; // class Mesh

#endif // Mesh_HPP_
