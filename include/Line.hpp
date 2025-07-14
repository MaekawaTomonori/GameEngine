#ifndef Line_HPP_
#define Line_HPP_

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "Math/Matrix.hpp"

class LineCommon;
class DirectXAdapter;
class CameraManager;

class Line {
public:
    struct LineInstance {
        Vector4 startPos;
        Vector4 endPos;
        Vector4 color;
    };

    struct TransformMatrix {
        Matrix4x4 viewProjection;
    };

private:
    struct VertexData {
        Vector4 position;
    };

    LineCommon* common_ = nullptr;
    DirectXAdapter* adapter_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
    CameraManager* cameraManager_ = nullptr;

    std::string uuid_;

    // 基本線形状用の頂点バッファ（2つの頂点）
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    VertexData* vertexData_ = nullptr;

    // インスタンスデータ用のバッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_;
    LineInstance* instanceData_ = nullptr;
    uint32_t maxInstances_ = 1000;
    uint32_t currentInstanceCount_ = 0;
    uint32_t instanceSrvIndex_ = 0;

    // 変換行列用のバッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
    TransformMatrix* transformData_ = nullptr;

    std::vector<LineInstance> lines_;

public:
    Line();
    ~Line();

    void Initialize();
    void Update();
    void Draw() const;

    // 線を追加
    void AddLine(const Vector3& start, const Vector3& end, const Vector4& color = {1.0f, 1.0f, 1.0f, 1.0f}, float thickness = 1.0f);
    
    // すべての線をクリア
    void Clear();

    // ビュープロジェクション行列を設定
    void SetViewProjectionMatrix(const Matrix4x4& viewProjection);

private:
    void CreateVertexBuffer();
    void CreateInstanceBuffer();
    void CreateTransformBuffer();
    void UpdateInstanceBuffer();
};

#endif // Line_HPP_
