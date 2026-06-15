#include "ProjectionMath.h"

#include "LGF/Math/MathConstants.h"

#include <cassert>
#include <cmath>

namespace LGF::Math {

	Matrix4x4 MakePerspectiveFovMatrix(
		float fovY,
		float aspectRatio,
		float nearClip,
		float farClip) {

		assert(std::fabs(aspectRatio) > EPSILON);
		assert(std::fabs(fovY) > 0.0f);
		assert(std::fabs(farClip - nearClip) > 0.0f);

		Matrix4x4 result{};

		const float cotY = 1.0f / std::tan(fovY * 0.5f);

		result.m[0][0] = cotY / aspectRatio;
		result.m[1][1] = cotY;

		result.m[2][2] = farClip / (farClip - nearClip);
		result.m[2][3] = 1.0f;

		result.m[3][2] =
			-nearClip * farClip / (farClip - nearClip);

		return result;
	}

	Matrix4x4 MakeOrthographicMatrix(
		float left,
		float top,
		float right,
		float bottom,
		float nearClip,
		float farClip) {

		assert(std::fabs(right - left) > 0.0f);
		assert(std::fabs(bottom - top) > 0.0f);
		assert(std::fabs(farClip - nearClip) > 0.0f);

		Matrix4x4 result{};

		result.m[0][0] = 2.0f / (right - left);
		result.m[1][1] = 2.0f / (top - bottom);
		result.m[2][2] = 1.0f / (farClip - nearClip);

		result.m[3][0] =
			(left + right) / (left - right);

		result.m[3][1] =
			(top + bottom) / (bottom - top);

		result.m[3][2] =
			nearClip / (nearClip - farClip);

		result.m[3][3] = 1.0f;

		return result;
	}

	Matrix4x4 MakeViewportMatrix(
		float left,
		float top,
		float width,
		float height,
		float minDepth,
		float maxDepth) {

		Matrix4x4 result{};

		result.m[0][0] = width * 0.5f;
		result.m[1][1] = -height * 0.5f;
		result.m[2][2] = maxDepth - minDepth;

		result.m[3][0] = left + width * 0.5f;
		result.m[3][1] = top + height * 0.5f;
		result.m[3][2] = minDepth;

		result.m[3][3] = 1.0f;

		return result;
	}
}