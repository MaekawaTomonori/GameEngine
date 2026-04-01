#ifndef Sprite_HPP_
#define Sprite_HPP_
#include "ReferencePtr.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"
#include <memory>

#include "src/DirectX/DirectXAdapter.hpp"

class SpriteCommon;

/** @brief 2Dスプライトクラス
 * テクスチャ付き2D四角形の描画を管理
 */
class Sprite {
    /** @brief スプライトのマテリアルデータ
     */
    struct Material {
        Vector4 color;
    };

    /** @brief スプライトの頂点データ
     */
    struct VertexData {
        Vector4 position;
        Vector2 uv;
    };

    /** @brief スプライトの変換行列データ
     */
    struct Transformation {
        Matrix4x4 wvp;
        Matrix4x4 world;
    };

    GESTD::ReferencePtr<SpriteCommon> common_;
    GESTD::ReferencePtr<DirectXAdapter> adapter_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

    std::string uuid_;

    std::string texturePath_;

    /** vertex resource
     */
    std::unique_ptr<DX12Resource> vr_;
    /** vertex buffer view
     */
    D3D12_VERTEX_BUFFER_VIEW vbv_{};
    /** vertex data
     */
    VertexData* vd_ = nullptr;

    /** index resource
     */
    std::unique_ptr<DX12Resource> ir_;
    /** index buffer view
     */
    D3D12_INDEX_BUFFER_VIEW ibv_{};
    uint32_t* index_ = nullptr;

    /** material resource
     */
    std::unique_ptr<DX12Resource> mr_;
    Material* material_ = nullptr;

    /** world transform
     */
    std::unique_ptr<DX12Resource> wr_;
    Transformation* wd_ = nullptr;

    Vector2 position_ = {0, 0};
    Vector2 size_ = {100, 100};

    float rotation_ = 0.f;

    Vector2 anchorPoint_ = {0.5f, 0.5f};
    bool flipX_ = false;
    bool flipY_ = false;

    Vector2 leftTop_{};
    Vector2 texSize_ = {100, 100};

    bool posteffect_ = false;

public:
    Sprite();
    ~Sprite();

    /** @brief スプライトを初期化
     * @param _texture テクスチャパス
     */
    void Initialize(const std::string&_texture);

    /** @brief スプライトの更新処理
     */
    void Update();

    /** @brief スプライトを描画
     */
    void Draw();

    /** @brief 位置を取得
     * @return 位置ベクトルへの参照
     */
    const Vector2& GetPosition() const;

    /** @brief 位置を設定
     * @param _p 位置ベクトル
     */
    void SetPosition(const Vector2& _p);

    /** @brief サイズを取得
     * @return サイズベクトルへの参照
     */
    const Vector2& GetSize() const;

    /** @brief サイズを設定
     * @param _s サイズベクトル
     */
    void SetSize(const Vector2& _s);

    /** @brief 回転角度を取得
     * @return 回転角度（ラジアン）
     */
    float GetRotation() const;

    /** @brief 回転角度を設定
     * @param _r 回転角度（ラジアン）
     */
    void SetRotation(float _r);

    /** @brief 色を取得
     * @return 色ベクトルへの参照
     */
    const Vector4& GetColor() const;

    /** @brief 色を設定
     * @param _color 色ベクトル
     */
    void SetColor(const Vector4& _color) const;

    /** @brief アンカーポイントを取得
     * @return アンカーポイントへの参照
     */
    const Vector2& GetAnchorPoint() const;

    /** @brief アンカーポイントを設定
     * @param _a アンカーポイント
     */
    void SetAnchorPoint(const Vector2& _a);

    /** @brief X軸反転の状態を取得
     * @return 反転している場合true
     */
    bool IsFlipX() const;

    /** @brief X軸反転を設定
     * @param _f 反転する場合true
     */
    void SetFlipX(bool _f);

    /** @brief Y軸反転の状態を取得
     * @return 反転している場合true
     */
    bool IsFlipY() const;

    /** @brief Y軸反転を設定
     * @param _f 反転する場合true
     */
    void SetFlipY(bool _f);

    /** @brief テクスチャの左上座標を取得
     * @return 左上座標への参照
     */
    const Vector2& GetTextureLeftTop() const;

    /** @brief テクスチャの左上座標を設定
     * @param _textureLeftTop 左上座標
     */
    void SetTextureLeftTop(const Vector2& _textureLeftTop);

    /** @brief テクスチャサイズを取得
     * @return テクスチャサイズへの参照
     */
    const Vector2& GetTextureSize() const;

    /** @brief テクスチャサイズを設定
     * @param _textureSize テクスチャサイズ
     */
    void SetTextureSize(const Vector2& _textureSize);

    /** @brief ポストエフェクトを有効化/無効化
     * @param _active アクティブにする場合true
     */
    void SetActivePostEffect(bool _active);

    /** @brief テクスチャを設定
     * @param _texture テクスチャパス
     */
    void SetTexture(const std::string& _texture);

private:
    /** @brief テクスチャサイズの調整
     */
    void AdjustTextureSize();

    /** @brief マップデータの更新（頂点・WVP行列）
     */
    void UpdateMapData() const;

    /** @brief デバッグ情報の表示
     */
    void Debug();
}; // class Sprite

#endif // Sprite_HPP_
