#ifndef LineInstance_HPP_
#define LineInstance_HPP_

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <memory>

#include "ReferencePtr.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

class LineCommon;
class DirectXAdapter;
class CameraController;

/** @brief 3Dラインの実体（Engine内部専用）
 * 生成・破棄は LineCommon が行い、外部からは公開ハンドルの Line 経由でのみ操作される。
 */
class LineInstance {
    /** @brief ラインの頂点データ
     */
    struct VertexData {
        Vector4 position;
    };

    /** @brief ラインのマテリアルデータ
     */
    struct Material {
        Vector4 color;
    };

    /** @brief ラインの変換行列データ
     */
    struct Transformation {
        Matrix4x4 WVP;
    };

    GESTD::ReferencePtr<LineCommon> common_;
    GESTD::ReferencePtr<DirectXAdapter> adapter_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
    GESTD::ReferencePtr<CameraController> cameraManager_;

    std::string uuid_;

    const uint32_t MAX_LINES = 1000;

    /** 基本線形状用の頂点バッファ（2つの頂点）
     */
    std::unique_ptr<DX12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    VertexData* vertexData_ = nullptr;

    /** Material
     */
    std::unique_ptr<DX12Resource> materialResource_;
    Material* materialData_ = nullptr;

    std::unique_ptr<DX12Resource> transformationResource_;
    Transformation* transformationData_ = nullptr;

    std::vector<Vector4> positions_;

    std::string name_;
    uint32_t id_ = 0;

    GESTD::LifetimeSentinel lifetime_;

public:
    LineInstance();
    ~LineInstance();

    /** @brief ラインを初期化
     */
    void Initialize();

    /** @brief ラインの更新処理
     */
    void Update();

    /** @brief ラインを描画キューに登録
     */
    void Draw();

    /** @brief 描画本体（LineCommonから直接・非virtualに呼ばれる） */
    void ExecuteDraw() const;

    /** @brief 線を追加
     * @param _start 始点座標
     * @param _end 終点座標
     */
    void AddLine(const Vector3& _start, const Vector3& _end);

    /** @brief すべての線をクリア
     */
    void Clear();

    /** @brief 色を設定
     * @param _color 色ベクトル
     */
    void SetColor(const Vector4& _color) const;

    /** @brief 名前を設定
     * @param _name 名前
     */
    void SetName(const std::string& _name);

    /** @brief 自身への安全な参照を取得する
     * LineCommon がハンドル（Line）生成時にのみ使用する。
     * @return ダングリング検出つきの参照
     */
    GESTD::ReferencePtr<LineInstance> GetReference() { return GESTD::ReferencePtr<LineInstance>(this, lifetime_); }

private:
    /** @brief 頂点バッファの生成
     */
    void CreateVertexBuffer();

    /** @brief マテリアルバッファの生成
     */
    void CreateMaterialBuffer();

    /** @brief 変換バッファの生成
     */
    void CreateTransformationBuffer();
};

#endif // LineInstance_HPP_
