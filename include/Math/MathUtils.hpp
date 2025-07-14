#pragma once
#include <cmath>

#include "Matrix.hpp"
#include "Transform.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"

//struct Transform;

namespace MathUtils{
	constexpr float F_PI = 3.14159265358979323846264338327950288f;
	constexpr double PI  = 3.14159265358979323846264338327950288;

    float Random(float min, float max);
    Vector3 Random(Vector3 min, Vector3 max);

    float Lerp(const float& a, const float& b, float t);

    Vector3 Lerp(const Vector3& a, const Vector3& b, float t);
    Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t);

    template<typename Type>
    Type Factorial(Type n) {
        if (n <= 0)return 1;

        return static_cast<Type>(n * Factorial(n - 1));
    }

    template <typename Type>
    Type Permutation(Type n, Type r) {
        return static_cast<Type>(Factorial(n) / Factorial(n - r));
    }

    float Distance(const Vector3& a, const Vector3& b);
    //Vector3 TransformNormal(const Vector3& v, const Transform& t);

    namespace Matrix {
        Matrix3x3 MakeIdentity3x3();

        Matrix4x4 MakeIdentity();
        Matrix4x4 MakeTranslateMatrix(const Vector3& velocity);
        Matrix4x4 MakeScaleMatrix(const Vector3& scale);
        Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
        Vector4 Transform(const Vector4& vector, const Matrix4x4& matrix);

        Matrix4x4 MakeRotateX(float rad);
        Matrix4x4 MakeRotateY(float rad);
        Matrix4x4 MakeRotateZ(float rad);
        Matrix4x4 MakeRotate(const Quaternion& rotate);
        Matrix4x4 MakeAffineMatrix(const ::Transform& transform);
        Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
        Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate);
        Matrix4x4 MakeAffineMatrix(const Matrix4x4& scale, const Matrix4x4& rotate, const Matrix4x4& translate);

        Matrix4x4 MakeOrthogonalMatrix(float left, float right, float top, float bottom, float znear, float zfar);
        Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
        Matrix4x4 MakeViewportMatrix(float width, float height, float top, float bottom, float depthMax, float depthMin);
    }
};

