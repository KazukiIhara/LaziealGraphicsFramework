#include "ShadowCascadeCalculator.h"

#include <algorithm>
#include <cmath>

#include "LGF/Graphics/Camera3D.h"
#include "LGF/Math/MathUtils.h"

using namespace LGF;

float ShadowCascadeCalculator::GetShadowFarClip(const Camera3D& camera) {
	const float nearClip = (std::max)(camera.param.nearClip, 0.01f);
	return (std::max)(nearClip + 0.01f, (std::min)(camera.param.farClip, kMaximumShadowDistance));
}

std::array<Vector3, 8> ShadowCascadeCalculator::CalculateFrustumCorners(
	const Camera3D& camera,
	float aspectRatio,
	float nearClip,
	float farClip) {
	const Quaternion rotation = Math::MakeQuaternion(Vector3(camera.pitch, camera.yaw, 0.0f));
	const Matrix4x4 cameraWorldMatrix = Math::MakeAffineMatrix(
		{ 1.0f, 1.0f, 1.0f },
		rotation,
		camera.translation);
	const Matrix4x4 viewProjection =
		Math::Inverse(cameraWorldMatrix) *
		Math::MakePerspectiveFovMatrix(camera.param.fovY, aspectRatio, nearClip, farClip);
	const Matrix4x4 inverseViewProjection = Math::Inverse(viewProjection);

	// DirectXのNDC範囲（XY: -1～1、Z: 0～1）から視錐台の8頂点を復元する。
	return {
		Math::Transform({ -1.0f,  1.0f, 0.0f }, inverseViewProjection),
		Math::Transform({  1.0f,  1.0f, 0.0f }, inverseViewProjection),
		Math::Transform({  1.0f, -1.0f, 0.0f }, inverseViewProjection),
		Math::Transform({ -1.0f, -1.0f, 0.0f }, inverseViewProjection),
		Math::Transform({ -1.0f,  1.0f, 1.0f }, inverseViewProjection),
		Math::Transform({  1.0f,  1.0f, 1.0f }, inverseViewProjection),
		Math::Transform({  1.0f, -1.0f, 1.0f }, inverseViewProjection),
		Math::Transform({ -1.0f, -1.0f, 1.0f }, inverseViewProjection),
	};
}

ShadowCascadeCalculator::CascadeFrusta ShadowCascadeCalculator::Calculate(
	const Camera3D& camera,
	float aspectRatio) {
	constexpr float kSplitLambda = 0.65f;
	const float nearClip = (std::max)(camera.param.nearClip, 0.01f);
	const float farClip = GetShadowFarClip(camera);
	CascadeFrusta cascades{};
	std::array<float, kCascadeCount + 1> splitDepths{};
	splitDepths[0] = nearClip;

	for (uint32_t cascadeIndex = 0; cascadeIndex < kCascadeCount; ++cascadeIndex) {
		const float splitRatio = static_cast<float>(cascadeIndex + 1) / static_cast<float>(kCascadeCount);
		const float uniformSplit = nearClip + (farClip - nearClip) * splitRatio;
		const float logarithmicSplit = nearClip * std::pow(farClip / nearClip, splitRatio);
		splitDepths[cascadeIndex + 1] = cascadeIndex + 1 == kCascadeCount
			? farClip
			: uniformSplit * (1.0f - kSplitLambda) + logarithmicSplit * kSplitLambda;
	}

	for (uint32_t cascadeIndex = 0; cascadeIndex < kCascadeCount; ++cascadeIndex) {
		const float logicalNear = splitDepths[cascadeIndex];
		const float cascadeFar = splitDepths[cascadeIndex + 1];
		const float blendOverlap = cascadeIndex == 0u
			? 0.0f
			: (logicalNear - splitDepths[cascadeIndex - 1]) * kCascadeBlendRatio;
		const float cascadeNear = logicalNear - blendOverlap;
		CascadeFrustum& cascade = cascades[cascadeIndex];
		cascade.nearClip = cascadeNear;
		cascade.farClip = cascadeFar;
		cascade.corners = CalculateFrustumCorners(camera, aspectRatio, cascadeNear, cascadeFar);
	}

	return cascades;
}
