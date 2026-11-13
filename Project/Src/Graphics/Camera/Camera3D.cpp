#include "LGF/Graphics/Camera3D.h"
#include "LGF/Graphics/Graphics3D.h"
#include "LGF/Graphics/Line3D.h"
#include "LGF/Math/MathUtils.h"

#include "Graphics/Shadow/ShadowCascadeCalculator.h"

#include <array>
#include <cstdint>

using namespace LGF;

namespace {
	void DrawFrustumEdges(const std::array<Vector3, 8>& corners, const Vector4& color) {
		constexpr std::array<std::array<uint32_t, 2>, 12> edges{ {
			{ { 0, 1 } }, { { 1, 2 } }, { { 2, 3 } }, { { 3, 0 } },
			{ { 4, 5 } }, { { 5, 6 } }, { { 6, 7 } }, { { 7, 4 } },
			{ { 0, 4 } }, { { 1, 5 } }, { { 2, 6 } }, { { 3, 7 } },
		} };

		for (const auto& edge : edges) {
			Line3D(corners[edge[0]], corners[edge[1]]).Draw(Math::Identity4x4(), color);
		}
	}
}

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

void Camera3D::SetShadowScene() const {
	Graphics3D::SetShadowCamera(*this);
}

void Camera3D::DrawDebug(const Camera3DDebugDraw& debugDraw) const {
	if (!debugDraw.showFrustum && !debugDraw.showShadowCascades) {
		return;
	}

	const float aspectRatio = param.useWindowAspectRatio
		? static_cast<float>(Graphics3D::GetWindowWidth()) /
			static_cast<float>(Graphics3D::GetWindowHeight())
		: param.aspectRatio;

	if (debugDraw.showFrustum) {
		DrawFrustumEdges(
			ShadowCascadeCalculator::CalculateFrustumCorners(
				*this,
				aspectRatio,
				param.nearClip,
				param.farClip),
			debugDraw.frustumColor);
	}

	if (debugDraw.showShadowCascades) {
		const auto cascades = ShadowCascadeCalculator::Calculate(*this, aspectRatio);
		for (uint32_t cascadeIndex = 0;
			cascadeIndex < ShadowCascadeCalculator::kCascadeCount;
			++cascadeIndex) {
			DrawFrustumEdges(
				cascades[cascadeIndex].corners,
				debugDraw.shadowCascadeColors[cascadeIndex]);
		}
	}
}

Vector3 Camera3D::ScreenToWorld(const Vector2& screenPosition, float depth) const {
	const float windowWidth = static_cast<float>(Graphics3D::GetWindowWidth());
	const float windowHeight = static_cast<float>(Graphics3D::GetWindowHeight());
	if (windowWidth <= 0.0f || windowHeight <= 0.0f) {
		return translation;
	}

	const Quaternion rotation =
		Math::MakeQuaternion(Vector3{ pitch, yaw, 0.0f });
	const Matrix4x4 worldMatrix = Math::MakeAffineMatrix(
		{ 1.0f, 1.0f, 1.0f },
		rotation,
		translation);
	const Matrix4x4 viewMatrix = Math::Inverse(worldMatrix);
	const float aspectRatio = param.useWindowAspectRatio
		? windowWidth / windowHeight
		: param.aspectRatio;
	const Matrix4x4 projectionMatrix = Math::MakePerspectiveFovMatrix(
		param.fovY,
		aspectRatio,
		param.nearClip,
		param.farClip);
	const Matrix4x4 inverseViewProjection =
		Math::Inverse(viewMatrix * projectionMatrix);

	const Vector3 normalizedDeviceCoordinate{
		screenPosition.x / windowWidth * 2.0f - 1.0f,
		1.0f - screenPosition.y / windowHeight * 2.0f,
		depth,
	};
	return Math::Transform(normalizedDeviceCoordinate, inverseViewProjection);
}
