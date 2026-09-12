#ifndef Line_HPP_
#define Line_HPP_

#include <string>

#include "ReferencePtr.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"

class LineInstance;

/** @brief 3Dラインレンダリングクラス（公開ハンドル）
 * デバッグ用の3D線分描画を提供。
 * 実体（LineInstance）はEngine側が所有し、本クラスは実体への安全な参照を保持するだけの薄いラッパー。
 * 実体が破棄された後に呼び出しても安全に無視される。
 */
class Line {
    GESTD::ReferencePtr<LineInstance> instance_;

public:
    Line();
    ~Line();

    Line(const Line&) = delete;
    Line& operator=(const Line&) = delete;

    Line(Line&& _other) noexcept;
    Line& operator=(Line&& _other) noexcept;

    /** @brief ラインを初期化
     */
    void Initialize();

    /** @brief ラインの更新処理
     */
    void Update();

    /** @brief ラインを描画
     */
    void Draw() const;

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
};

#endif // Line_HPP_
