#ifndef Skybox_HPP_
#define Skybox_HPP_
#include <memory>
#include <string>

#include "Common/SkyCommon.hpp"
#include "Math/Vector4.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

class Skybox {
    struct VertexData {
        Vector4 position;
    };

    SkyCommon* common_ = nullptr;
    DirectXAdapter* adapter_ = nullptr;

    std::string texture_;

    std::unique_ptr<DX12Resource> vr_;
    D3D12_VERTEX_BUFFER_VIEW vbv_{};
    VertexData* vd_ = nullptr;

    std::unique_ptr<DX12Resource> ir_;
    D3D12_INDEX_BUFFER_VIEW ibv_{};
    uint32_t* id_ = nullptr;

public:
    void Initialize(const std::string& _texture);

private:
    void CreateVertex();
    void CreateIndex();
}; // class Skybox

#endif // Skybox_HPP_
