#include "Math/MathUtils.hpp"
#include <random>

Matrix3x3 MathUtils::Matrix::MakeIdentity3x3() {
    return Matrix3x3 {
        1,0,0,
        0,1,0,
        0,0,1
    };
}

Matrix4x4 MathUtils::Matrix::MakeIdentity() {
    return Matrix4x4{
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
}
Matrix4x4 MathUtils::Matrix::MakeTranslateMatrix(const Vector3& velocity) {
    return Matrix4x4 {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        velocity.x, velocity.y, velocity.z, 1
    };
}

Matrix4x4 MathUtils::Matrix::MakeScaleMatrix(const Vector3& scale) {
    return Matrix4x4 {
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1
    };
}

//Vector3 MathUtils::Matrix::Transform(const Vector3& vector, const Matrix4x4& matrix) {
//    Vector3 v = {
//        vector.x * matrix.matrix[0][0] + vector.y * matrix.matrix[1][0] + vector.z * matrix.matrix[2][0] + 1 * matrix.matrix[3][0],
//        vector.x * matrix.matrix[0][1] + vector.y * matrix.matrix[1][1] + vector.z * matrix.matrix[2][1] + 1 * matrix.matrix[3][1],
//        vector.x * matrix.matrix[0][2] + vector.y * matrix.matrix[1][2] + vector.z * matrix.matrix[2][2] + 1 * matrix.matrix[3][2]
//    };
//    const float w = vector.x * matrix.matrix[0][3] + vector.y * matrix.matrix[1][3] + vector.z * matrix.matrix[2][3] + 1 * matrix.matrix[3][3];
//    assert(w != 0);
//    if (w != 1){
//        v.x /= w;
//        v.y /= w;
//        v.z /= w;
//    }
//    return v;
//}

Matrix4x4 MathUtils::Matrix::MakeRotateX(const float rad) {
    return {
        1, 0, 0, 0,
        0, std::cosf(rad), std::sinf(rad), 0,
        0, -std::sinf(rad), std::cosf(rad), 0,
        0, 0, 0, 1
    };
}

Matrix4x4 MathUtils::Matrix::MakeRotateY(float rad) {
    return Matrix4x4 {
        std::cosf(rad), 0, -std::sinf(rad), 0,
        0, 1, 0, 0,
        std::sinf(rad), 0, std::cosf(rad), 0,
        0, 0, 0, 1
    };
}

Matrix4x4 MathUtils::Matrix::MakeRotateZ(const float rad) {
    return Matrix4x4 {
        std::cosf(rad), std::sinf(rad), 0, 0,
        -std::sinf(rad), std::cosf(rad), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

//Matrix4x4 MathUtils::Matrix::MakeAffineMatrix(const ::Transform& transform) {
//    return MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
//}

Matrix4x4 MathUtils::Matrix::MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
    Matrix4x4 scaleMat = MakeScaleMatrix(scale);
    Matrix4x4 rotateMatX = MakeRotateX(rotate.x);
    Matrix4x4 rotateMatY = MakeRotateY(rotate.y);
    Matrix4x4 rotateMatZ = MakeRotateZ(rotate.z);
    Matrix4x4 rotateMat = rotateMatX * rotateMatY * rotateMatZ;
    Matrix4x4 translateMat = MakeTranslateMatrix(translate);
    return scaleMat * rotateMat * translateMat;
}

Matrix4x4 MathUtils::Matrix::MakeAffineMatrix(const Matrix4x4& scale, const Matrix4x4& rotate,
	const Matrix4x4& translate) {
    return scale * rotate * translate;
}

Matrix4x4 MathUtils::Matrix::MakeOrthogonalMatrix(float left, float right, float top, float bottom, float znear, float zfar) {
    return Matrix4x4 {
        2 / (right - left), 0, 0, 0,
        0, 2 / (top - bottom), 0, 0,
        0, 0, 1 / (zfar - znear), 0,
        (left + right) / (left - right), (top + bottom) / (bottom - top), znear / (znear - zfar), 1
    };
}

Matrix4x4 MathUtils::Matrix::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
    return Matrix4x4 {
        1 / aspectRatio * (1 / tanf(fovY / 2)), 0, 0, 0,
        0, 1 / tanf(fovY / 2), 0, 0,
        0,0, farClip / (farClip - nearClip), 1,
        0, 0, -(nearClip * farClip) / (farClip - nearClip), 0,
    };
}

Matrix4x4 MathUtils::Matrix::MakeViewportMatrix(float left, float right, float top, float bottom, float depthMax,
                                                float depthMin) {
    return Matrix4x4 {
        (right - left) / 2, 0, 0, 0,
        0, -(top - bottom) / 2, 0, 0,
        0, 0, depthMax - depthMin, 0,
        left + (right - left) / 2, top + (top - bottom) / 2, depthMin, 1
    };
}

float MathUtils::Random(float min, float max) {
	std::random_device seed;
    std::mt19937 random(seed());

    std::uniform_real_distribution<float> dist(min, max);
    return dist(random);
}

float MathUtils::Distance(const Vector3& a, const Vector3& b) {
    return std::sqrtf(std::powf(a.x - b.x, 2) + std::powf(a.y - b.y, 2) + std::powf(a.z - b.z, 2));
}

//Vector3 MathUtils::TransformNormal(const Vector3& v, const Transform& t) {
//    Matrix4x4 m = Matrix::MakeAffineMatrix(t);
//    return {
//        v.x * m.matrix[0][0] + v.y * m.matrix[1][0] + v.z * m.matrix[2][0],
//        v.x * m.matrix[0][1] + v.y * m.matrix[1][1] + v.z * m.matrix[2][1],
//        v.x * m.matrix[0][2] + v.y * m.matrix[1][2] + v.z * m.matrix[2][2],
//    };
//}
