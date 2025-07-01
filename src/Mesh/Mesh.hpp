#ifndef Mesh_HPP_
#define Mesh_HPP_
#include <d3d12.h>
#include <string>
#include <wrl/client.h>

#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "src/DirectX/DirectXAdapter.hpp"

class Mesh {
    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };

    struct Material {
        
    };

    struct MaterialData {
        std::string texture;
    };

    struct ModelData {
        std::vector<VertexData> vertices;
        MaterialData material;
    };

    DirectXAdapter* adapter_ = nullptr;

    // vertex resource
    Microsoft::WRL::ComPtr<ID3D12Resource> vr_;
    // vertex buffer view
    D3D12_VERTEX_BUFFER_VIEW vbv_ {};
    // vertex data
    VertexData* vd_ = nullptr;

    // index resource
    Microsoft::WRL::ComPtr<ID3D12Resource> ir_;
    // index buffer view
    D3D12_INDEX_BUFFER_VIEW ibv_ {};
    uint32_t* index_ = nullptr;

    // material resource
    Microsoft::WRL::ComPtr<ID3D12Resource> mr_;
    Material* material_ = nullptr;

    ModelData modelData_;
public:
    void Initialize(DirectXAdapter* _adapter, const std::string &_directory, const std::string &_name);
    void Update();
    void Draw();

private:
    void LoadFile(const std::string &_directory, const std::string &_name);

    void LoadObj(const std::string& _directory, const std::string& _name);
    //void LoadGltf(const char* _fileName);

    MaterialData LoadMaterialTemplateFile(std::string& _directory, std::string& _name);
}; // class Mesh

#endif // Mesh_HPP_
