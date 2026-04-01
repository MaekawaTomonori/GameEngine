#ifndef Skybox_HPP_
#define Skybox_HPP_
#include <memory>
#include <string>

#include "WeakPtr.hpp"
#include "Common/SkyCommon.hpp"
#include "Math/Matrix.hpp"
#include "Math/Transform.hpp"
#include "Math/Vector4.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

class Skybox {
    struct VertexData {
        Vector4 position;
    };

    struct Transformation {
        Matrix4x4 wvp;
    };

    struct Material {
        Vector4 color;
    };

    GESTD::WeakPtr<SkyCommon> common_;
    GESTD::WeakPtr<DirectXAdapter> adapter_ = nullptr;

    std::string texture_;

    std::unique_ptr<DX12Resource> vr_;
    D3D12_VERTEX_BUFFER_VIEW vbv_{};
    VertexData* vd_ = nullptr;

    std::unique_ptr<DX12Resource> ir_;
    D3D12_INDEX_BUFFER_VIEW ibv_{};
    uint32_t* id_ = nullptr;

    std::unique_ptr<DX12Resource> wr_;
    Transformation* wd_ = nullptr;

    std::unique_ptr<DX12Resource> mr_;
    Material* md_ = nullptr;

    Transform transform_ {};

    bool posteffect_ = true;

public:
    void Initialize(const std::string& _texture);
    void Update();
    void Draw();

    void SetColor(const Vector4& _color) const;

private:
    void CreateVertex();
    void CreateIndex();
}; // class Skybox

#endif // Skybox_HPP_
