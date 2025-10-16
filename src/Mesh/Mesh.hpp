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

/// <summary>
/// メッシュクラス
/// 頂点データ、インデックス、マテリアルを管理
/// </summary>
class Mesh {
    /// <summary>
    /// メッシュのマテリアルデータ
    /// </summary>
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
    /// <summary>
    /// メッシュを初期化
    /// </summary>
    /// <param name="_adapter">DirectXアダプター</param>
    /// <param name="_name">メッシュ名</param>
    /// <param name="_raw">メッシュデータ</param>
    void Initialize(DirectXAdapter* _adapter, const std::string &_name, const MeshData& _raw);

    /// <summary>
    /// メッシュの更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// メッシュを描画
    /// </summary>
    void Draw() const;

    /// <summary>
    /// デバッグ情報の表示
    /// </summary>
    void Debug();

    /// <summary>
    /// 頂点バッファビューを設定
    /// </summary>
    /// <param name="_vbv">頂点バッファビュー</param>
    void SetVBV(D3D12_VERTEX_BUFFER_VIEW _vbv);

    /// <summary>
    /// メッシュデータを取得
    /// </summary>
    /// <returns>メッシュデータ</returns>
    MeshData GetData() const;

    /// <summary>
    /// テクスチャを設定
    /// </summary>
    /// <param name="_texturePath">テクスチャパス</param>
    void SetTexture(const std::string& _texturePath);
}; // class Mesh

#endif // Mesh_HPP_
