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
class CameraController;

/// <summary>
/// 3Dラインレンダリングクラス
/// デバッグ用の3D線分描画を提供
/// </summary>
class Line {
    /// <summary>
    /// ラインの頂点データ
    /// </summary>
    struct VertexData {
        Vector4 position;
    };

    /// <summary>
    /// ラインのマテリアルデータ
    /// </summary>
    struct Material {
        Vector4 color;
    };

    /// <summary>
    /// ラインの変換行列データ
    /// </summary>
    struct Transformation {
        Matrix4x4 WVP;
    };

    LineCommon* common_ = nullptr;
    DirectXAdapter* adapter_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
    CameraController* cameraManager_ = nullptr;

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

    /// <summary>
    /// ラインを初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// ラインの更新処理
    /// </summary>
    void Update() const;

    /// <summary>
    /// ラインを描画
    /// </summary>
    void Draw() const;

    /// <summary>
    /// 線を追加
    /// </summary>
    /// <param name="start">始点座標</param>
    /// <param name="end">終点座標</param>
    void AddLine(const Vector3& start, const Vector3& end);

    /// <summary>
    /// すべての線をクリア
    /// </summary>
    void Clear();

    /// <summary>
    /// 色を設定
    /// </summary>
    /// <param name="color">色ベクトル</param>
    void SetColor(Vector4 color) const;

private:
    /// <summary>
    /// 頂点バッファの生成
    /// </summary>
    void CreateVertexBuffer();

    /// <summary>
    /// マテリアルバッファの生成
    /// </summary>
    void CreateMaterialBuffer();

    /// <summary>
    /// 変換バッファの生成
    /// </summary>
    void CreateTransformationBuffer();
};

#endif // Line_HPP_
