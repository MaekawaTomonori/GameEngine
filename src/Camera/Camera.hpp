#ifndef Camera_HPP_
#define Camera_HPP_
#include <string>

#include "Math/Matrix.hpp"
#include "Math/Transform.hpp"
#include "Math/Vector3.hpp"

/// <summary>
/// GPU転送用のカメラデータ
/// </summary>
struct CameraForGpu {
    Vector3 position;
    float pad;
};

/// <summary>
/// カメラクラス
/// ビュー行列と投影行列を管理
/// </summary>
class Camera {
public:
    Transform transform_{};

private:
    std::string uuid_;

    Matrix4x4 matrix_{};
    Matrix4x4 view_{};
    Matrix4x4 projection_{};

    float fov_ = 0.45f;
    float aspectRatio_ = .0f;

    float near_ = 0.1f;
    float far_ = 100.0f;

public:
    Camera();

    /// <summary>
    /// カメラを初期化
    /// </summary>
    /// <param name="_ratio">アスペクト比</param>
    void Initialize(float _ratio);

    /// <summary>
    /// カメラの更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// デバッグ情報の表示
    /// </summary>
    void Debug();

    /// <summary>
    /// ユニークIDを取得
    /// </summary>
    /// <returns>ユニークID文字列</returns>
    std::string GetUniqueId();

    /// <summary>
    /// ビュー行列を取得
    /// </summary>
    /// <returns>ビュー行列への参照</returns>
    const Matrix4x4 &GetView() const { return view_; }

    /// <summary>
    /// 投影行列を取得
    /// </summary>
    /// <returns>投影行列への参照</returns>
    const Matrix4x4 &GetProjection() const { return projection_; }

    /// <summary>
    /// カメラ行列を取得
    /// </summary>
    /// <returns>カメラ行列への参照</returns>
    const Matrix4x4 &GetMatrix() const { return matrix_; }

    /// <summary>
    /// ビュー投影行列を取得
    /// </summary>
    /// <returns>ビュー投影行列</returns>
    Matrix4x4 GetViewProjection() const;

    /// <summary>
    /// GPU転送用カメラデータを取得
    /// </summary>
    /// <returns>GPU用カメラデータ</returns>
    CameraForGpu GetCameraForGpu() const;

    /// <summary>
    /// 視野角を設定
    /// </summary>
    /// <param name="fov">視野角（ラジアン）</param>
    void SetFov(float fov) { fov_ = fov; }

    /// <summary>
    /// アスペクト比を設定
    /// </summary>
    /// <param name="aspectRatio">アスペクト比</param>
    void SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }

    /// <summary>
    /// ニアクリップ平面を設定
    /// </summary>
    /// <param name="nearPlane">ニアクリップ距離</param>
    void SetNear(float nearPlane) { near_ = nearPlane; }

    /// <summary>
    /// ファークリップ平面を設定
    /// </summary>
    /// <param name="farPlane">ファークリップ距離</param>
    void SetFar(float farPlane) { far_ = farPlane; }
}; // class Camera

#endif // Camera_HPP_
