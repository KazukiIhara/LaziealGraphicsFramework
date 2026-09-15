#include "LGF/Graphics/Camera3D.h"
#include "LGF/Graphics/Graphics3D.h"
#include "LGF/Graphics/Line3D.h"
#include "LGF/Math/MathUtils.h"

#include <array>
#include <cstdint>

using namespace LGF;

Camera3D::Camera3D() {
}

Camera3D::Camera3D(Vector3 translation, float yaw, float pitch, const Param& param) :
	translation(translation), yaw(yaw), pitch(pitch), param(param) {
}

Camera3D::~Camera3D() = default;

void Camera3D::SetScene() const {
	Graphics3D::SetCamera(*this);
}

void Camera3D::SetCullingScene() const {
	Graphics3D::SetCullingCamera(*this);
}

void Camera3D::DrawDebug(const Camera3DDebugDraw& debugDraw) const {
	if (!debugDraw.showFrustum) {
		return;
	}

	const Quaternion rotation = Math::MakeQuaternion(Vector3(pitch, yaw, 0.0f));
	const Matrix4x4 cameraWorldMatrix = Math::MakeAffineMatrix(
		{ 1.0f, 1.0f, 1.0f },
		rotation,
		translation);
	const float aspectRatio = param.useWindowAspectRatio
		? static_cast<float>(Graphics3D::GetWindowWidth()) /
			static_cast<float>(Graphics3D::GetWindowHeight())
		: param.aspectRatio;
	const Matrix4x4 viewProjection =
		Math::Inverse(cameraWorldMatrix) *
		Math::MakePerspectiveFovMatrix(
			param.fovY,
			aspectRatio,
			param.nearClip,
			param.farClip);
	const Matrix4x4 inverseViewProjection = Math::Inverse(viewProjection);

	// DirectXのNDC範囲（XY: -1～1、Z: 0～1）から視錐台の8頂点を復元する。
	const std::array<Vector3, 8> corners{
		Math::Transform({ -1.0f,  1.0f, 0.0f }, inverseViewProjection),
		Math::Transform({  1.0f,  1.0f, 0.0f }, inverseViewProjection),
		Math::Transform({  1.0f, -1.0f, 0.0f }, inverseViewProjection),
		Math::Transform({ -1.0f, -1.0f, 0.0f }, inverseViewProjection),
		Math::Transform({ -1.0f,  1.0f, 1.0f }, inverseViewProjection),
		Math::Transform({  1.0f,  1.0f, 1.0f }, inverseViewProjection),
		Math::Transform({  1.0f, -1.0f, 1.0f }, inverseViewProjection),
		Math::Transform({ -1.0f, -1.0f, 1.0f }, inverseViewProjection),
	};
	constexpr std::array<std::array<uint32_t, 2>, 12> edges{ {
		{ { 0, 1 } }, { { 1, 2 } }, { { 2, 3 } }, { { 3, 0 } },
		{ { 4, 5 } }, { { 5, 6 } }, { { 6, 7 } }, { { 7, 4 } },
		{ { 0, 4 } }, { { 1, 5 } }, { { 2, 6 } }, { { 3, 7 } },
	} };

	for (const auto& edge : edges) {
		Line3D(corners[edge[0]], corners[edge[1]]).Draw(
			Math::Identity4x4(),
			debugDraw.frustumColor);
	}
}
