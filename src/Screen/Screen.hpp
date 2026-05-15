#ifndef Screen_HPP_
#define Screen_HPP_

class Screen {
    float width_  = 1280.f;
    float height_ = 720.f;

public:
    Screen() = default;
    Screen(const float _w, const float _h) : width_(_w), height_(_h) {}

    void Resize(const float _w, const float _h) {
        width_  = _w;
        height_ = _h;
    }

    [[nodiscard]] float Width()  const { return width_; }
    [[nodiscard]] float Height() const { return height_; }

}; // class Screen

#endif // Screen_HPP_
