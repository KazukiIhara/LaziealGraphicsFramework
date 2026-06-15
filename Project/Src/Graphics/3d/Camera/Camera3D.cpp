#include "Camera3D.h"

#include "LGF/Math/MathUtils.h"

using namespace LGF;

Camera3D::Camera3D() {
}

Camera3D::~Camera3D() = default;

void Camera3D::Initialize() {
	Update();
}

void Camera3D::Update() {
	// 回転を作る
	rotation_ = Math::EulerToQuaternionYXZ({
		pitch_,
		yaw_,
		0.0f
		});

	// カメラのワールド行列を作る
	const Matrix4x4 worldMatrix =
		Math::MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f },
			rotation_,
			translation_);

	// View行列を作る
	viewMatrix_ = Math::Inverse(worldMatrix);

	// Projection行列を作る
	projectionMatrix_ =
		Math::MakePerspectiveFovMatrix(
			fovY_,
			aspectRatio_,
			nearClip_,
			farClip_);
}

void Camera3D::SetTranslation(const Vector3& translation) {
	translation_ = translation;
}

void Camera3D::SetYaw(float yaw) {
	yaw_ = yaw;
}

void Camera3D::SetPitch(float pitch) {
	pitch_ = pitch;
}

const Matrix4x4 Camera3D::GetViewMatrix() const {
	return viewMatrix_;
}

const Matrix4x4 Camera3D::GetProjectionMatrix() const {
	return projectionMatrix_;
}

const Vector3 Camera3D::GetTranslation() const {
	return translation_;
}
