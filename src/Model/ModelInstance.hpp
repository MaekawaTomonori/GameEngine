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

/** @brief 3Dモデルの実体の共通基底（Engine内部専用）
 * トランスフォーム・メッシュ・ワールド行列バッファなど、モデル種別によらず共通の状態のみを持つ。
 * 種別固有の処理（Update/Draw/Debug）は派生クラスが実装する。
 * 生成・破棄は ModelCommon が行い、外部からは公開ハンドルの Model 経由でのみ操作される。
 */
class ModelInstance {
protected:
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

    std::unique_ptr<DX12Resource> wr_;
    Transformation* wd_ = nullptr;

    std::string environmentTexture_ = "";

    std::string canvasName_ = "Main";
    bool castShadow_ = true;

    GESTD::LifetimeSentinel lifetime_;

    /** @brief 派生クラス共通の初期化処理（モデルデータ読み込み・メッシュ生成・更新/デバッグ/シャドウ登録） */
    void InitializeCommon(const std::string& _name);

    void DebugTransformSection();
    void DebugMeshSection();

public:
    ModelInstance();
    virtual ~ModelInstance();

    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void Debug() = 0;

    void UpdateMapData() const;

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
}; // class ModelInstance

#endif // ModelInstance_HPP_
