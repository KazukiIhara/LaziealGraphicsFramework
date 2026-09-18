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
