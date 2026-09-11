#ifndef ModelInstance_HPP_
#define ModelInstance_HPP_
#include <functional>
#include <memory>

#include "ReferencePtr.hpp"

#include "Math/Matrix.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Mesh/Mesh.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

class ModelCommon;
struct ModelData;
class SkinningState;

/** @brief 3Dモデルの実体（Engine内部専用）
 * メッシュ、テクスチャ、アニメーション、スキニングを管理する。
 * 生成・破棄は ModelCommon が行い、外部からは公開ハンドルの Model 経由でのみ操作される。
 */
class ModelInstance {
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

    /** スキニング専用の状態（骨格・アニメーション・スキンクラスター）
     * スキニングデータを持たないモデルではnullptrのまま
     */
    std::unique_ptr<SkinningState> skinning_;

    /** GPU RESOURCES
     */
    /** world transform
     */
    std::unique_ptr<DX12Resource> wr_;
    Transformation* wd_ = nullptr;

    std::function<void()> drawCommand_;

    std::string environmentTexture_ = "";

    std::string canvasName_ = "Main";
    bool castShadow_ = true;

    GESTD::LifetimeSentinel lifetime_;

public:
    ModelInstance();
    ~ModelInstance();

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

    ModelInstance& SetName(const std::string& _name);
    ModelInstance& SetTranslate(const Vector3& _translate);
    ModelInstance& SetRotate(const Vector3& _rotate);
    ModelInstance& SetScale(const Vector3& _scale);
    ModelInstance& SetEnvironmentTexture(const std::string& _texture);
    ModelInstance& SetTexture(const std::string& _texture);
    ModelInstance& SetTilingMultiply(Vector2 _mul);
    ModelInstance& SetColor(const Vector4& _color);
    ModelInstance& SetCanvasName(const std::string& _canvasName);

    const std::string& GetCanvasName() const { return canvasName_; }
    const std::string& GetName() const;

    /** @brief 自身への安全な参照を取得する
     * ModelCommon がハンドル（Model）生成時にのみ使用する。
     * @return ダングリング検出つきの参照
     */
    GESTD::ReferencePtr<ModelInstance> GetReference() { return GESTD::ReferencePtr<ModelInstance>(this, lifetime_); }

private:
    /** @brief デバッグ情報の表示
     */
    void Debug();

    /** @brief デバッグ用ラインの描画（スキニングモデルのみ）
     */
    void DrawLine() const;
}; // class ModelInstance

#endif // ModelInstance_HPP_
