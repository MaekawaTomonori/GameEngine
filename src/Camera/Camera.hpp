#ifndef Camera_HPP_
#define Camera_HPP_
#include <string>

#include "Math/Matrix.hpp"
#include "Math/Transform.hpp"
#include "Math/Vector3.hpp"

/** @brief GPU転送用のカメラデータ
 **/
struct CameraForGpu {
    Vector3 position;
    float pad;
};

/** @brief カメラクラス
 ** ビュー行列と投影行列を管理
 **/
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

    /** @brief カメラを初期化
     ** @param _ratio アスペクト比
     **/
    void Initialize(float _ratio);

    /** @brief カメラの更新処理
     **/
    void Update();

    /** @brief デバッグ情報の表示
     **/
    void Debug();

    /** @brief ユニークIDを取得
     ** @return ユニークID文字列
     **/
    std::string GetUniqueId();

    /** @brief ビュー行列を取得
     ** @return ビュー行列への参照
     **/
    const Matrix4x4 &GetView() const { return view_; }

    /** @brief 投影行列を取得
     ** @return 投影行列への参照
     **/
    const Matrix4x4 &GetProjection() const { return projection_; }

    /** @brief カメラ行列を取得
     ** @return カメラ行列への参照
     **/
    const Matrix4x4 &GetMatrix() const { return matrix_; }

    /** @brief ビュー投影行列を取得
     ** @return ビュー投影行列
     **/
    Matrix4x4 GetViewProjection() const;

    /** @brief GPU転送用カメラデータを取得
     ** @return GPU用カメラデータ
     **/
    CameraForGpu GetCameraForGpu() const;

    /** @brief 視野角を取得
     ** @return 視野角（ラジアン）
     **/
    float GetFov() const;

    /** @brief 視野角を設定
     ** @param _fov 視野角（ラジアン）
     **/
    void SetFov(const float _fov) { fov_ = _fov; }

    /** @brief アスペクト比を設定
     ** @param _aspectRatio アスペクト比
     **/
    void SetAspectRatio(const float _aspectRatio) { aspectRatio_ = _aspectRatio; }

    /** @brief ニアクリップ平面を設定
     ** @param _nearPlane ニアクリップ距離
     **/
    void SetNear(const float _nearPlane) { near_ = _nearPlane; }

    /** @brief ファークリップ平面を設定
     ** @param _farPlane ファークリップ距離
     **/
    void SetFar(const float _farPlane) { far_ = _farPlane; }

    void LookAt(const Vector3& _position);
}; // class Camera

#endif // Camera_HPP_
