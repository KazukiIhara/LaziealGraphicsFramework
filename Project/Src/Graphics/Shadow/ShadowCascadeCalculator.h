#pragma once

#include <array>
#include <cstdint>

#include "LGF/Math/Vector3.h"

namespace LGF {

	class Camera3D;

	namespace ShadowCascadeCalculator {

		inline constexpr uint32_t kCascadeCount = 4;
		inline constexpr float kMaximumShadowDistance = 100.0f;
		inline constexpr float kCascadeBlendRatio = 0.1f;

		struct CascadeFrustum {
			float nearClip = 0.0f;
			float farClip = 0.0f;
			std::array<Vector3, 8> corners{};
		};

		using CascadeFrusta = std::array<CascadeFrustum, kCascadeCount>;

		float GetShadowFarClip(const Camera3D& camera);
		std::array<Vector3, 8> CalculateFrustumCorners(
			const Camera3D& camera,
			float aspectRatio,
			float nearClip,
			float farClip);
		CascadeFrusta Calculate(const Camera3D& camera, float aspectRatio);

	}

}
