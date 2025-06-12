#pragma once

struct Matrix3x3{
    float matrix[3][3];
};

struct Matrix4x4{
    float matrix[4][4];

    Matrix4x4 operator+(const Matrix4x4& other) const {
        return Matrix4x4 {
            matrix[0][0] + other.matrix[0][0],
            matrix[0][1] + other.matrix[0][1],
            matrix[0][2] + other.matrix[0][2],
            matrix[0][3] + other.matrix[0][3],
            matrix[1][0] + other.matrix[1][0],
            matrix[1][1] + other.matrix[1][1],
            matrix[1][2] + other.matrix[1][2],
            matrix[1][3] + other.matrix[1][3],
            matrix[2][0] + other.matrix[2][0],
            matrix[2][1] + other.matrix[2][1],
            matrix[2][2] + other.matrix[2][2],
            matrix[2][3] + other.matrix[2][3],
            matrix[3][0] + other.matrix[3][0],
            matrix[3][1] + other.matrix[3][1],
            matrix[3][2] + other.matrix[3][2],
            matrix[3][3] + other.matrix[3][3]
        };
    }
    Matrix4x4 operator-(const Matrix4x4& other) const {
        return Matrix4x4 {
            matrix[0][0] - other.matrix[0][0],
            matrix[0][1] - other.matrix[0][1],
            matrix[0][2] - other.matrix[0][2],
            matrix[0][3] - other.matrix[0][3],
            matrix[1][0] - other.matrix[1][0],
            matrix[1][1] - other.matrix[1][1],
            matrix[1][2] - other.matrix[1][2],
            matrix[1][3] - other.matrix[1][3],
            matrix[2][0] - other.matrix[2][0],
            matrix[2][1] - other.matrix[2][1],
            matrix[2][2] - other.matrix[2][2],
            matrix[2][3] - other.matrix[2][3],
            matrix[3][0] - other.matrix[3][0],
            matrix[3][1] - other.matrix[3][1],
            matrix[3][2] - other.matrix[3][2],
            matrix[3][3] - other.matrix[3][3]
        };
    }
    Matrix4x4 operator*(const float& other) const {
        return Matrix4x4 {
            matrix[0][0] * other,
            matrix[0][1] * other,
            matrix[0][2] * other,
            matrix[0][3] * other,
            matrix[1][0] * other,
            matrix[1][1] * other,
            matrix[1][2] * other,
            matrix[1][3] * other,
            matrix[2][0] * other,
            matrix[2][1] * other,
            matrix[2][2] * other,
            matrix[2][3] * other,
            matrix[3][0] * other,
            matrix[3][1] * other,
            matrix[3][2] * other,
            matrix[3][3] * other
        };
    }
    Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 m {};
        for (int i = 0; i < 4; ++i){
            for (int j = 0; j < 4; ++j){
                for (int k = 0; k < 4; ++k){
                    m.matrix[i][j] += matrix[i][k] * other.matrix[k][j];
                }
            }
        }
        return m;
    }
    Matrix4x4 operator/(const float& other) const {
        return Matrix4x4 {
            matrix[0][0] / other,
            matrix[0][1] / other,
            matrix[0][2] / other,
            matrix[0][3] / other,
            matrix[1][0] / other,
            matrix[1][1] / other,
            matrix[1][2] / other,
            matrix[1][3] / other,
            matrix[2][0] / other,
            matrix[2][1] / other,
            matrix[2][2] / other,
            matrix[2][3] / other,
            matrix[3][0] / other,
            matrix[3][1] / other,
            matrix[3][2] / other,
            matrix[3][3] / other
        };
    }
    Matrix4x4 Inverse() const {
        float a = matrix[0][0] * matrix[1][1] * matrix[2][2] * matrix[3][3] + matrix[0][0] * matrix[1][2] * matrix[2][3] * matrix[3][1] + matrix[0][0] * matrix[1][3] * matrix[2][1] * matrix[3][2] - matrix[0][0] * matrix[1][3] * matrix[2][2] * matrix[3][1] - matrix[0][0] * matrix[1][2] * matrix[2][1] * matrix[3][3] - matrix[0][0] * matrix[1][1] * matrix[2][3] * matrix[3][2] - matrix[0][1] * matrix[1][0] * matrix[2][2] * matrix[3][3] - matrix[0][2] * matrix[1][0] * matrix[2][3] * matrix[3][1] - matrix[0][3] * matrix[1][0] * matrix[2][1] * matrix[3][2] + matrix[0][3] * matrix[1][0] * matrix[2][2] * matrix[3][1] + matrix[0][2] * matrix[1][0] * matrix[2][1] * matrix[3][3] + matrix[0][1] * matrix[1][0] * matrix[2][3] * matrix[3][2] + matrix[0][1] * matrix[1][2] * matrix[2][0] * matrix[3][3] + matrix[0][2] * matrix[1][3] * matrix[2][0] * matrix[3][1] + matrix[0][3] * matrix[1][1] * matrix[2][0] * matrix[3][2] - matrix[0][3] * matrix[1][2] * matrix[2][0] * matrix[3][1] - matrix[0][2] * matrix[1][1] * matrix[2][0] * matrix[3][3] - matrix[0][1] * matrix[1][3] * matrix[2][0] * matrix[3][2] - matrix[0][1] * matrix[1][2] * matrix[2][3] * matrix[3][0] - matrix[0][2] * matrix[1][3] * matrix[2][1] * matrix[3][0] - matrix[0][3] * matrix[1][1] * matrix[2][2] * matrix[3][0] + matrix[0][3] * matrix[1][2] * matrix[2][1] * matrix[3][0] + matrix[0][2] * matrix[1][1] * matrix[2][3] * matrix[3][0] + matrix[0][1] * matrix[1][3] * matrix[2][2] * matrix[3][0];
        return Matrix4x4 {
            (matrix[1][1] * matrix[2][2] * matrix[3][3] + matrix[1][2] * matrix[2][3] * matrix[3][1] + matrix[1][3] * matrix[2][1] * matrix[3][2] - matrix[1][3] * matrix[2][2] * matrix[3][1] - matrix[1][2] * matrix[2][1] * matrix[3][3] - matrix[1][2] * matrix[2][3] * matrix[3][2]),
            (-matrix[0][1] * matrix[2][2] * matrix[3][3] - matrix[0][2] * matrix[2][3] * matrix[3][1] - matrix[0][3] * matrix[2][1] * matrix[3][2] + matrix[0][3] * matrix[2][2] * matrix[3][1] + matrix[0][2] * matrix[2][1] * matrix[3][3] + matrix[0][1] * matrix[2][3] * matrix[3][2]),
            (matrix[0][1] * matrix[1][2] * matrix[3][3] + matrix[0][2] * matrix[1][3] * matrix[3][1] + matrix[0][3] * matrix[1][1] * matrix[3][2] - matrix[0][3] * matrix[1][2] * matrix[3][1] - matrix[0][2] * matrix[1][1] * matrix[3][3] - matrix[0][1] * matrix[1][3] * matrix[3][2]),
            (-matrix[0][1] * matrix[1][2] * matrix[2][3] - matrix[0][2] * matrix[1][3] * matrix[2][1] - matrix[0][3] * matrix[1][1] * matrix[2][2] + matrix[0][3] * matrix[1][2] * matrix[2][1] + matrix[0][2] * matrix[1][1] * matrix[2][3] + matrix[0][1] * matrix[1][3] * matrix[2][2]),

            (-matrix[1][0] * matrix[2][2] * matrix[3][3] - matrix[1][2] * matrix[2][3] * matrix[3][0] - matrix[1][3] * matrix[2][0] * matrix[3][2] + matrix[1][3] * matrix[2][2] * matrix[3][0] + matrix[1][2] * matrix[2][0] * matrix[3][3] + matrix[1][0] * matrix[2][3] * matrix[3][2]),
            (matrix[0][0] * matrix[2][2] * matrix[3][3] + matrix[0][2] * matrix[2][3] * matrix[3][0] + matrix[0][3] * matrix[2][0] * matrix[3][2] - matrix[0][3] * matrix[2][2] * matrix[3][0] - matrix[0][2] * matrix[2][0] * matrix[3][3] - matrix[0][0] * matrix[2][3] * matrix[3][2]),
            (-matrix[0][0] * matrix[1][2] * matrix[3][3] - matrix[0][2] * matrix[1][3] * matrix[3][0] - matrix[0][3] * matrix[1][0] * matrix[3][2] + matrix[0][3] * matrix[1][2] * matrix[3][0] * matrix[0][2] * matrix[1][0] * matrix[3][3] + matrix[0][0] * matrix[1][3] * matrix[3][2]),
            (matrix[0][0] * matrix[1][2] * matrix[2][3] + matrix[0][2] * matrix[1][3] * matrix[2][0] + matrix[0][3] * matrix[1][0] * matrix[2][2] - matrix[0][3] * matrix[1][2] * matrix[2][0] - matrix[0][2] * matrix[1][0] * matrix[2][3] - matrix[0][0] * matrix[1][3] * matrix[2][2]),

            (matrix[1][0] * matrix[2][1] * matrix[3][3] + matrix[1][1] * matrix[2][3] * matrix[3][0] + matrix[1][3] * matrix[2][0] * matrix[3][1] - matrix[1][3] * matrix[2][1] * matrix[3][0] - matrix[1][1] * matrix[2][0] * matrix[3][3] - matrix[1][0] * matrix[2][3] * matrix[3][1]),
            (-matrix[0][0] * matrix[2][1] * matrix[3][3] - matrix[0][1] * matrix[2][3] * matrix[3][0] - matrix[0][3] * matrix[2][0] * matrix[3][1] + matrix[0][3] * matrix[2][1] * matrix[3][0] + matrix[0][1] * matrix[2][0] * matrix[3][3] + matrix[0][0] * matrix[2][3] * matrix[3][1]),
            (matrix[0][0] * matrix[1][1] * matrix[3][3] + matrix[0][1] * matrix[1][3] * matrix[3][0] + matrix[0][3] * matrix[1][0] * matrix[3][1] - matrix[0][3] * matrix[1][1] * matrix[3][0] - matrix[0][1] * matrix[1][0] * matrix[3][3] - matrix[0][0] * matrix[1][3] * matrix[3][1]),
            (-matrix[0][0] * matrix[1][1] * matrix[2][3] - matrix[0][1] * matrix[1][3] * matrix[2][0] - matrix[0][3] * matrix[1][0] * matrix[2][1] + matrix[0][3] * matrix[1][1] * matrix[2][0] + matrix[0][1] * matrix[1][0] * matrix[2][3] + matrix[0][0] * matrix[1][3] * matrix[2][1]),

            (-matrix[1][0] * matrix[2][1] * matrix[3][2] - matrix[1][1] * matrix[2][2] * matrix[3][0] - matrix[1][2] * matrix[2][0] * matrix[3][1] + matrix[1][2] * matrix[2][1] * matrix[3][0] + matrix[1][1] * matrix[2][0] * matrix[3][2] + matrix[1][0] * matrix[2][2] * matrix[3][1]),
            (matrix[0][0] * matrix[2][1] * matrix[3][2] + matrix[0][1] * matrix[2][2] * matrix[3][0] + matrix[0][2] * matrix[2][0] * matrix[3][1] - matrix[0][2] * matrix[2][1] * matrix[3][0] - matrix[0][1] * matrix[2][0] * matrix[3][2] - matrix[0][0] * matrix[2][2] * matrix[3][1]),
            (-matrix[0][0] * matrix[1][1] * matrix[3][2] - matrix[0][1] * matrix[1][2] * matrix[3][0] - matrix[0][2] * matrix[1][0] * matrix[3][1] + matrix[0][2] * matrix[1][1] * matrix[3][0] + matrix[0][1] * matrix[1][0] * matrix[3][2] + matrix[0][0] * matrix[1][2] * matrix[3][1]),
            (matrix[0][0] * matrix[1][1] * matrix[2][2] + matrix[0][1] * matrix[1][2] * matrix[2][0] + matrix[0][2] * matrix[1][0] * matrix[2][1] - matrix[0][2] * matrix[1][1] * matrix[2][0] - matrix[0][1] * matrix[1][0] * matrix[2][2] - matrix[0][0] * matrix[1][1] * matrix[2][1])
        } / a;
    }

    Matrix4x4 Transpose() const {
        Matrix4x4 m {};

        for (int row = 0; row < 4; ++row){
            for (int column = 0; column < 4; ++column){
                m.matrix[row][column] = matrix[column][row];
            }
        }

        return m;
    }
};