#ifndef ScreenUnits_HPP_
#define ScreenUnits_HPP_
#include "src/Screen/Screen.hpp"
#include "Pattern/Singleton.hpp"

namespace Screen::Utils {

    class vw {
        float value_;
    public:
        explicit vw(long double v)
            : value_(static_cast<float>(
                static_cast<long double>(Singleton<Screen>::GetInstance()->Width()) * v / 100.0L)) {}
        operator float() const { return value_; }
    };

    class vh {
        float value_;
    public:
        explicit vh(long double v)
            : value_(static_cast<float>(
                static_cast<long double>(Singleton<Screen>::GetInstance()->Height()) * v / 100.0L)) {}
        operator float() const { return value_; }
    };

    class px {
        float value_;
    public:
        explicit px(long double v) : value_(static_cast<float>(v)) {}
        operator float() const { return value_; }
    };

} // namespace Screen::Utils

inline Screen::Utils::vw operator""_vw(long double v)        { return Screen::Utils::vw(v); }
inline Screen::Utils::vh operator""_vh(long double v)        { return Screen::Utils::vh(v); }
inline Screen::Utils::px operator""_px(long double v)        { return Screen::Utils::px(v); }
inline Screen::Utils::vw operator""_vw(unsigned long long v) { return Screen::Utils::vw(static_cast<long double>(v)); }
inline Screen::Utils::vh operator""_vh(unsigned long long v) { return Screen::Utils::vh(static_cast<long double>(v)); }
inline Screen::Utils::px operator""_px(unsigned long long v) { return Screen::Utils::px(static_cast<long double>(v)); }

#endif // ScreenUnits_HPP_
