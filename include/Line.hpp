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

/** @brief 3Dラインレンダリングクラス
 ** デバッグ用の3D線分描画を提供
 **/
class Line {
    /** @brief ラインの頂点データ
     **/
    struct VertexData {
        Vector4 position;
    };

    /** @brief ラインのマテリアルデータ
     **/
    struct Material {
        Vector4 color;
    };

    /** @brief ラインの変換行列データ
     **/
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

    std::string name_;
    uint32_t id_ = 0;

public:
    Line();
    ~Line();

    /** @brief ラインを初期化
     **/
    void Initialize();

    /** @brief ラインの更新処理
     **/
    void Update();

    /** @brief ラインを描画
     **/
    void Draw() const;

    /** @brief 線を追加
     ** @param _start 始点座標
     ** @param _end 終点座標
     **/
    void AddLine(const Vector3& _start, const Vector3& _end);

    /** @brief すべての線をクリア
     **/
    void Clear();

    /** @brief 色を設定
     ** @param _color 色ベクトル
     **/
    void SetColor(Vector4 _color) const;

    /** @brief 名前を設定
     ** @param _name 名前
     **/
    void SetName(const std::string& _name);

private:
    /** @brief 頂点バッファの生成
     **/
    void CreateVertexBuffer();

    /** @brief マテリアルバッファの生成
     **/
    void CreateMaterialBuffer();

    /** @brief 変換バッファの生成
     **/
    void CreateTransformationBuffer();
};

#endif // Line_HPP_
