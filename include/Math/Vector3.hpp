#pragma once
#include <cmath>

#include "Vector2.hpp"

/// <summary>
/// 3Dベクトル構造体
/// 3D座標と基本的なベクトル演算を提供
/// </summary>
struct Vector3{
    float x, y, z;

    Vector3 operator+(const Vector3& v) const {
        return {x + v.x, y + v.y, z + v.z};
    }

	void operator+=(const Vector3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
	void operator+=(const Vector2& v) {
		x += v.x;
		y += v.y;
		z += 0;
	}

    Vector3 operator-(const Vector3& v) const {
        return {x - v.x, y - v.y, z - v.z};
    }

    Vector3 operator*(const float f) const {
        return {x * f, y * f, z * f};
    }

    void operator*=(const float f) {
        x *= f;
        y *= f;
        z *= f;
    }

    void operator*=(const Vector3& v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
    }

	Vector3 operator/(const float& f) const {
        return {x / f, y / f, z / f};
    }

	float Length() const {
		return sqrtf(x * x + y * y + z * z);
	}

	Vector3 Normalize() const {
        const float len = Length();
        if (len <= 0.0000001f) return *this;

		return Vector3{
			x / len,
			y / len,
			z / len
		};
	}

	static Vector3 Random();
};
