#include "Screen.hpp"

#include <algorithm>

#undef min
#undef max

void Screen::Resize(const float _w, const float _h) {
    width_  = _w;
    height_ = _h;

    if (!hasReference_) {
        referenceWidth_  = _w;
        referenceHeight_ = _h;
        hasReference_    = true;
    }

    scale_ = std::min(width_ / referenceWidth_, height_ / referenceHeight_);
}
