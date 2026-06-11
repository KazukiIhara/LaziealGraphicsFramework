#include "Camera3D.h"

#include "LGF/Math/MathUtilities.h"

using namespace LGF;

Camera3D::Camera3D() {
}

Camera3D::~Camera3D() = default;

void Camera3D::Update() {
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

const Matrix4x4 LGF::Camera3D::GetViewMatrix() const {
	return viewMatrix_;
}

const Matrix4x4 LGF::Camera3D::GetProjectionMatrix() const {
	return projectionMatrix_;
}
