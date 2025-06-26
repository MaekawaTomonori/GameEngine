#pragma once
#include <cmath>

struct Vector2{
    float x, y;

    Vector2 operator+(const Vector2& v) const {
        return { x + v.x, y + v.y };
    }

    Vector2 operator-(const Vector2& v) const {
        return { x - v.x, y - v.y };
    }

    Vector2 operator*(const Vector2& v) const {
		return { x * v.x, y * v.y };
    }

    Vector2 operator*(const float f) const {
        return { x * f, y * f };
    }

	void operator+=(const Vector2& v) {
		x += v.x;
		y += v.y;
	}

    bool operator==(const Vector2& v) const {
        return x == v.x && y == v.y;
    }

    float Length() const {
        return sqrtf(x * x + y * y);
    }

    void Normalize() {
        if (Length() <= 0.0000001f)return;
        float length = Length();
        x /= length;
        y /= length;
    }

    static Vector2 Random();
};

