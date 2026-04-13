#ifndef Text_HPP_
#define Text_HPP_

#include <string>

#include "ReferencePtr.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"

class TextCommon;

/** @brief テキスト描画オブジェクト
 *
 * 文字列・位置・サイズ・色を保持し、Draw() を呼ぶとその
 * フレームの描画キューへ追加される。
 *
 * ## 使い方
 * @code
 *   // クラスメンバーとして宣言
 *   Text label_;
 *
 *   // 初期化（一度だけ）
 *   label_.Initialize("Score: 0", 20.f, 20.f, 32.f);
 *
 *   // 毎フレーム Draw() を呼ぶ
 *   label_.SetText("Score: " + std::to_string(score_));
 *   label_.Draw();
 * @endcode
 *
 * @note フォントは Engine 側で自動ロードされる。
 *       Draw() で積んだ内容は次のフレームの Update() でリセットされる。
 */
class Text {
    GESTD::ReferencePtr<TextCommon> common_;

    std::string text_     = "";
    Vector2     position_ = {};
    float       fontSize_ = 32.f;
    Vector4     color_    = {1.f, 1.f, 1.f, 1.f};
    bool        visible_  = true;

public:
    Text();
    ~Text();

    /** @brief テキストの初期設定
     * @param _text     表示する文字列
     * @param _x        スクリーン左端 X 座標（ピクセル）
     * @param _y        テキスト上端 Y 座標（ピクセル）
     * @param _fontSize フォントサイズ（ピクセル高さ）
     */
    void Initialize(const std::string& _text,
                    float _x = 0.f, float _y = 0.f,
                    float _fontSize = 32.f);

    /** @brief このフレームの描画キューに追加する
     *
     * visible_ が false の場合は何もしない。
     * フォントが未ロードの場合は何もしない（エラーにならない）。
     */
    void Draw();

    // ─── Setters ──────────────────────────────────────────────────

    /** @brief 表示文字列を変更 */
    void SetText(const std::string& _text);

    /** @brief 位置を変更 */
    void SetPosition(float _x, float _y);

    /** @brief フォントサイズを変更 */
    void SetFontSize(float _fontSize);

    /** @brief RGBA カラーを変更（各要素 0.0–1.0）*/
    void SetColor(const Vector4& _color);

    /** @brief 表示 / 非表示を切り替え */
    void SetVisible(bool _visible);

    // ─── Getters ──────────────────────────────────────────────────

    const std::string& GetText()     const { return text_; }
    Vector2            GetPosition() const { return position_; }
    float              GetFontSize() const { return fontSize_; }
    const Vector4&     GetColor()    const { return color_; }
    bool               IsVisible()   const { return visible_; }
};

#endif // Text_HPP_
