#ifndef Sprite_HPP_
#define Sprite_HPP_
#include <string>

#include "ReferencePtr.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"

class SpriteInstance;

/** @brief 2Dスプライトクラス（公開ハンドル）
 * 実体（SpriteInstance）はEngine側が所有し、本クラスは実体への安全な参照を保持するだけの薄いラッパー。
 * 実体が破棄された後に呼び出しても安全に無視される。
 */
class Sprite {
    GESTD::ReferencePtr<SpriteInstance> instance_;

public:
    Sprite();
    ~Sprite();

    Sprite(const Sprite&) = delete;
    Sprite& operator=(const Sprite&) = delete;

    Sprite(Sprite&& _other) noexcept;
    Sprite& operator=(Sprite&& _other) noexcept;

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

    /** @brief 位置を基準解像度前提にする（自動スケール対象にする）か設定
     * @param _fixed 対象にする場合true
     */
    void SetFixed(bool _fixed);
}; // class Sprite

#endif // Sprite_HPP_
