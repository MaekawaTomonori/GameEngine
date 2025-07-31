#ifndef Line_HPP_
#define Line_HPP_

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <memory>

#include "Math/Matrix.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

class LineCommon;
class DirectXAdapter;
class CameraManager;

class Line {
    struct VertexData {
        Vector4 position;
    };

    struct Material {
        Vector4 color;
    };

    struct Transformation {
        Matrix4x4 WVP;
    };

    LineCommon* common_ = nullptr;
    DirectXAdapter* adapter_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
    CameraManager* cameraManager_ = nullptr;

    std::string uuid_;

    const uint32_t MAX_LINES = 1000;

    // 基本線形状用の頂点バッファ（2つの頂点）
    std::unique_ptr<DX12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    VertexData* vertexData_ = nullptr;

    // Material
    std::unique_ptr<DX12Resource> materialResource_;
    Material* materialData_ = nullptr;

    std::unique_ptr<DX12Resource> transformationResource_;
    Transformation* transformationData_ = nullptr;

    std::vector<Vector4> positions_;

public:
    Line();
    ~Line();

    void Initialize();
    void Update() const;
    void Draw() const;

    // 線を追加
    void AddLine(const Vector3& start, const Vector3& end);
    
    // すべての線をクリア
    void Clear();

    void SetColor(Vector4 color) const;

private:
    void CreateVertexBuffer();
    void CreateMaterialBuffer();
    void CreateTransformationBuffer();
};

#endif // Line_HPP_
