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

/** @brief メッシュクラス
 * 頂点データ、インデックス、マテリアルを管理
 */
class Mesh {
    /** @brief メッシュのマテリアルデータ
     */
    struct Material {
        Vector4 color;           // 16 bytes (aligned)
        uint32_t lighting;       // 4 bytes
        float shininess;         // 4 bytes
        float coefficient;       // 4 bytes
        float pad1;              // 4 bytes (padding to 16-byte _boundary)
        Vector2 tilingMul;       // 8 bytes
        Vector2 pad2;            // 8 bytes (padding to 16-byte _boundary)
        Matrix4x4 uvTransform;   // 64 bytes (aligned)
    };

    GESTD::ReferencePtr<DirectXAdapter> adapter_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    std::string name_;

    MeshData data_;

    /** vertex resource
     */
    std::unique_ptr<DX12Resource> vr_;
    /** vertex buffer view
     */
    D3D12_VERTEX_BUFFER_VIEW vbv_ {};
    /** vertex data
     */
    Vertex* vd_ = nullptr;

    std::vector<D3D12_VERTEX_BUFFER_VIEW> vbvs_;

    /** index resource
     */
    std::unique_ptr<DX12Resource> ir_;
    /** index buffer view
     */
    D3D12_INDEX_BUFFER_VIEW ibv_ {};
    /** index
     */
    uint32_t* id_ = nullptr;

    /** material resource
     */
    std::unique_ptr<DX12Resource> mr_;
    /** material
     */
    Material* material_ = nullptr;

    /** current texture
     */
    std::string texture_;

    /** lighting
     */
	bool lighting_ = false;

    /** tiling aspect ratio lock
     */
    bool aspectRatioLocked_ = false;
    float aspectRatio_ = 1.0f;

public:
    /** @brief メッシュを初期化
     * @param _adapter DirectXアダプター
     * @param _name メッシュ名
     * @param _raw メッシュデータ
     */
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const std::string& _name, const MeshData& _raw);

    /** @brief メッシュの更新処理
     */
    void Update();

    /** @brief メッシュを描画
     */
    void Draw(uint16_t _instanceCount = 1) const;

    /** @brief ジオメトリのみを描画（シャドウパス用）
     *  ルートパラメータを一切設定しない。呼び出し元が事前に設定すること。
     */
    void DrawGeometryOnly() const;

    /** @brief デバッグ情報の表示
     */
    void Debug();

    /** @brief 頂点バッファビューを設定
     * @param _vbv 頂点バッファビュー
     */
    void SetVBV(const D3D12_VERTEX_BUFFER_VIEW& _vbv);

    /** @brief メッシュデータを取得
     * @return メッシュデータ
     */
    MeshData GetData() const;

    /** @brief テクスチャを設定
     * @param _texturePath テクスチャパス
     */
    void SetTexture(const std::string& _texturePath);

    /** @brief テクスチャのサイズを設定
     * @param _tilingMul テクスチャのタイリング倍率
     */
    void SetTextureSize(Vector2 _tilingMul) const;

    /** @brief 色の設定
     * @param _color 変更後の色
     */
    void SetColor(const Vector4& _color) const;

    void EnableLighting(bool _active);
}; // class Mesh

#endif // Mesh_HPP_
