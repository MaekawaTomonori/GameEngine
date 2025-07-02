#ifndef Camera_HPP_
#define Camera_HPP_
#include <string>

#include "Math/Matrix.hpp"
#include "Math/Transform.hpp"
#include "Math/Vector3.hpp"

struct CameraForGpu {
	Vector3 position;
};

class Camera {
    std::string uuid_;

    Transform transform_{};

    Matrix4x4 matrix_{};
    Matrix4x4 view_{};
    Matrix4x4 projection_{};

    float fov_ = 0.45f;
    float aspectRatio_ = .0f;

    float near_ = 0.1f;
    float far_ = 1000.0f;

public:
	Camera();

    void Initialize(float _ratio);
	void Update();
	void Debug();

	const Matrix4x4 &GetView() const { return view_; }
	const Matrix4x4 &GetProjection() const { return projection_; }
	const Matrix4x4 &GetMatrix() const { return matrix_; }
	void SetFov(float fov) { fov_ = fov; }
	void SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }
	void SetNear(float nearPlane) { near_ = nearPlane; }
	void SetFar(float farPlane) { far_ = farPlane; }

}; // class Camera

#endif // Camera_HPP_
