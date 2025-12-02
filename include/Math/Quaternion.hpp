#ifndef Quaternion_HPP_
#define Quaternion_HPP_
#include <cmath>

/** @brief クォータニオン構造体
 ** 3D回転を表現し、ジンバルロックを回避
 **/
struct Quaternion{
    float x, y, z, w;

    Quaternion operator+(const Quaternion& q) const {
        return { x + q.x, y + q.y, z + q.z, w + q.w };
    }

    Quaternion operator*(const Quaternion& q) const {
        return {
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w,
            w * q.w - x * q.x - y * q.y - z * q.z
        };
    }

    Quaternion operator*(const float s) const {
        return { x * s, y * s, z * s, w * s };
    }

    Quaternion operator/(const float s) const {
        return { x / s, y / s, z / s, w / s };
    }

    static Quaternion Identity() {
        return { 0, 0, 0, 1 };
    }

    Quaternion Conjugate() const {
        return { -x, -y, -z, w };
    }

    float Norm() const {
        return sqrtf(x * x + y * y + z * z + w * w);
    }

    Quaternion Normalize() const {
        return *this / Norm();
    }

    Quaternion Inverse() const {
        return Conjugate() / (x * x + y * y + z * z + w * w);
    }
};
#endif // Quaternion_HPP_
