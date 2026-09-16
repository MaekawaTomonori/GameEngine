#ifndef Screen_HPP_
#define Screen_HPP_

/** @brief 現在のウィンドウ解像度と、起動時解像度（基準解像度）に対する倍率を管理する
 */
class Screen {
    float width_  = 1280.f;
    float height_ = 720.f;

    /** @brief 基準解像度（最初のResize()呼び出し時の解像度） */
    float referenceWidth_  = 1280.f;
    float referenceHeight_ = 720.f;
    bool  hasReference_    = false;

    /** @brief 基準解像度に対する現在解像度の倍率（アスペクト比維持） */
    float scale_ = 1.f;

public:
    Screen() = default;
    Screen(const float _w, const float _h) : width_(_w), height_(_h) {}

    void Resize(const float _w, const float _h);

    [[nodiscard]] float Width()  const { return width_; }
    [[nodiscard]] float Height() const { return height_; }

    /** @brief 基準解像度に対する現在解像度の倍率を取得
     * @return 倍率
     */
    [[nodiscard]] float GetScale() const { return scale_; }

}; // class Screen

#endif // Screen_HPP_
