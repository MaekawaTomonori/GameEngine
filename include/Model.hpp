#ifndef Model_HPP_
#define Model_HPP_
#include <array>
#include <span>
#include <memory>

#include "ReferencePtr.hpp"

#include "Line.hpp"
#include "Math/Matrix.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Mesh/Mesh.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

const uint32_t MAX_INFLUENCE = 4;

/** @brief 3Dモデルクラス
 * メッシュ、テクスチャ、アニメーション、スキニングを管理
 */
class Model {
    /** @brief 頂点のスキニング影響情報
     */
    struct VertexInfluence {
        std::array<float, MAX_INFLUENCE> weights;
        std::array<int32_t, MAX_INFLUENCE> jointIndices;
    };

    /** @brief GPU用のスキニングデータ
     */
    struct WellForGpu {
        Matrix4x4 space;
        Matrix4x4 inverseTranspose;
    };

    /** @brief スキンクラスターデータ
     * アニメーション用のボーン変換情報を保持
     */
    struct SkinCluster {
        std::vector<Matrix4x4> inverseBindPoses;

        std::unique_ptr<DX12Resource> influenceResource;
        D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
        std::span<VertexInfluence> mappedInfluence;

        std::unique_ptr<DX12Resource> paletteResource;
        std::span<WellForGpu> mappedPalette;
        uint32_t srvIndex;
        std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteHandle;
    };

    /** @brief モデルの変換行列データ
     */
    struct Transformation {
        Matrix4x4 wvp;
        Matrix4x4 world;
        Matrix4x4 inverse;
    };

    GESTD::ReferencePtr<ModelCommon> common_;
    GESTD::ReferencePtr<DirectXAdapter> adapter_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

    std::string name_;
    std::string uuid_;
    Transform transform_;
    GESTD::ReferencePtr<ModelData> data_ = nullptr;
    std::unique_ptr<Mesh> mesh_;

    std::optional<Skeleton> pose_;

    bool animationEnable_ = true;
    bool animationTimerLock_ = true;
    float animationTime_ = 0.0f;

    Line line_;

    /** GPU RESOURCES
     */
    /** world transform
     */
    std::unique_ptr<DX12Resource> wr_;
    Transformation* wd_ = nullptr;

    /** Camera
     */
    std::unique_ptr<DX12Resource> cr_;
    CameraForGpu* cd_ = nullptr;

    SkinCluster skinCluster_;

    std::string environmentTexture_ = "";

    bool posteffect_ = true;
    bool castShadow_ = true;

public:
    Model();
    ~Model();

    /** @brief モデルを初期化
     * @param _name モデル名
     */
    void Initialize(const std::string& _name);

    /** @brief モデルの更新処理
     */
    void Update();

    /** @brief マップデータの更新
     */
    void UpdateMapData() const;

    /** @brief モデルを描画
     */
    void Draw() const;

    /** @brief 名前を設定
     * @param _name 
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetName(const std::string& _name);

    /** @brief 平行移動を設定
     * @param _translate 平行移動ベクトル
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetTranslate(const Vector3& _translate);

    /** @brief 回転を設定
     * @param _rotate 回転ベクトル
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetRotate(const Vector3& _rotate);

    /** @brief スケールを設定
     * @param _scale スケールベクトル
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetScale(const Vector3& _scale);

    /** @brief 環境マッピング用テクスチャを設定
     * @param _texture テクスチャパス
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetEnvironmentTexture(const std::string& _texture);

    /** @brief テクスチャを設定
     * @param _texture テクスチャパス
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetTexture(const std::string& _texture);

    /** @brief テクスチャのタイリング倍率を設定
     * @param _mul タイリング倍率
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetTilingMultiply(Vector2 _mul);

    /** @brief 色の設定
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetColor(const Vector4& _color);

    /** @brief モデル名の取得
     * @return モデル名
     */
    const std::string& GetName() const;

    /** @brief メッシュの取得
     * @return メッシュへのポインタ
     */
    GESTD::ReferencePtr<Mesh> GetMesh() const;

    /** @brief モデルデータを事前読み込み
     * @param _name モデル名
     */
    static void Load(const std::string& _name);

private:
    /** @brief デバッグ情報の表示
     */
    void Debug();

    /** @brief スキンクラスターの生成
     */
    void CreateSkinCluster();

    /** @brief バインドポーズの設定
     * @param _skeleton スケルトン
     */
    void SetBindPose(Skeleton& _skeleton);

    /** @brief スキンクラスターの更新
     */
    void UpdateSkinCluster();

    /** @brief スケルトンの更新
     */
    void UpdateSkeleton();

    /** @brief アニメーションの更新
     */
    void UpdateAnimation();

    /** @brief アニメーションの適用
     */
    void ApplyAnimation();

    /** @brief デバッグ用ラインの生成
     */
    void CreateLine();

    /** @brief デバッグ用ラインの描画
     */
    void DrawLine() const;
}; // class Model

#endif // Model_HPP_
