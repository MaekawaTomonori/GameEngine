#ifndef Sprite_HPP_
#define Sprite_HPP_
#include "Math/Matrix.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"
#include <memory>

#include "src/DirectX/DirectXAdapter.hpp"

class SpriteCommon;

/// <summary>
/// 2Dスプライトクラス
/// テクスチャ付き2D四角形の描画を管理
/// </summary>
class Sprite {
    /// <summary>
    /// スプライトのマテリアルデータ
    /// </summary>
    struct Material {
        Vector4 color;
    };

    /// <summary>
    /// スプライトの頂点データ
    /// </summary>
    struct VertexData {
        Vector4 position;
        Vector2 uv;
    };

    /// <summary>
    /// スプライトの変換行列データ
    /// </summary>
    struct Transformation {
        Matrix4x4 wvp;
        Matrix4x4 world;
    };

    SpriteCommon* common_ = nullptr;
    DirectXAdapter* adapter_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

    std::string uuid_;

    std::string texturePath_;

    // vertex resource
    std::unique_ptr<DX12Resource> vr_;
    // vertex buffer view
    D3D12_VERTEX_BUFFER_VIEW vbv_{};
    // vertex data
    VertexData* vd_ = nullptr;

    // index resource
    std::unique_ptr<DX12Resource> ir_;
    // index buffer view
    D3D12_INDEX_BUFFER_VIEW ibv_{};
    uint32_t* index_ = nullptr;

    // material resource
    std::unique_ptr<DX12Resource> mr_;
    Material* material_ = nullptr;

    // world transform
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


public:
    Sprite();

    /// <summary>
    /// スプライトを初期化
    /// </summary>
    /// <param name="_texture">テクスチャパス</param>
    void Initialize(const std::string&_texture);

    /// <summary>
    /// スプライトの更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// スプライトを描画
    /// </summary>
    void Draw();

    /// <summary>
    /// 位置を取得
    /// </summary>
    /// <returns>位置ベクトルへの参照</returns>
    const Vector2& GetPosition() const;

    /// <summary>
    /// 位置を設定
    /// </summary>
    /// <param name="p">位置ベクトル</param>
    void SetPosition(const Vector2& p);

    /// <summary>
    /// サイズを取得
    /// </summary>
    /// <returns>サイズベクトルへの参照</returns>
    const Vector2& GetSize() const;

    /// <summary>
    /// サイズを設定
    /// </summary>
    /// <param name="s">サイズベクトル</param>
    void SetSize(const Vector2& s);

    /// <summary>
    /// 回転角度を取得
    /// </summary>
    /// <returns>回転角度（ラジアン）</returns>
    float GetRotation() const;

    /// <summary>
    /// 回転角度を設定
    /// </summary>
    /// <param name="r">回転角度（ラジアン）</param>
    void SetRotation(float r);

    /// <summary>
    /// 色を取得
    /// </summary>
    /// <returns>色ベクトルへの参照</returns>
    const Vector4& GetColor() const;

    /// <summary>
    /// 色を設定
    /// </summary>
    /// <param name="color">色ベクトル</param>
    void SetColor(const Vector4& color) const;

    /// <summary>
    /// アンカーポイントを取得
    /// </summary>
    /// <returns>アンカーポイントへの参照</returns>
    const Vector2& GetAnchorPoint() const;

    /// <summary>
    /// アンカーポイントを設定
    /// </summary>
    /// <param name="a">アンカーポイント</param>
    void SetAnchorPoint(const Vector2& a);

    /// <summary>
    /// X軸反転の状態を取得
    /// </summary>
    /// <returns>反転している場合true</returns>
    bool IsFlipX() const;

    /// <summary>
    /// X軸反転を設定
    /// </summary>
    /// <param name="f">反転する場合true</param>
    void SetFlipX(bool f);

    /// <summary>
    /// Y軸反転の状態を取得
    /// </summary>
    /// <returns>反転している場合true</returns>
    bool IsFlipY() const;

    /// <summary>
    /// Y軸反転を設定
    /// </summary>
    /// <param name="f">反転する場合true</param>
    void SetFlipY(bool f);

    /// <summary>
    /// テクスチャの左上座標を取得
    /// </summary>
    /// <returns>左上座標への参照</returns>
    const Vector2& GetTextureLeftTop() const;

    /// <summary>
    /// テクスチャの左上座標を設定
    /// </summary>
    /// <param name="textureLeftTop">左上座標</param>
    void SetTextureLeftTop(const Vector2& textureLeftTop);

    /// <summary>
    /// テクスチャサイズを取得
    /// </summary>
    /// <returns>テクスチャサイズへの参照</returns>
    const Vector2& GetTextureSize() const;

    /// <summary>
    /// テクスチャサイズを設定
    /// </summary>
    /// <param name="textureSize">テクスチャサイズ</param>
    void SetTextureSize(const Vector2& textureSize);

private:
    /// <summary>
    /// テクスチャサイズの調整
    /// </summary>
    void AdjustTextureSize();

    /// <summary>
    /// デバッグ情報の表示
    /// </summary>
    void Debug();
}; // class Sprite

#endif // Sprite_HPP_
