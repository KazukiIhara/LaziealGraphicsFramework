#include "TransformMath.h"

#include "MatrixMath.h"

#include <cmath>

namespace LGF::Math {

	Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
		Matrix4x4 result = Identity4x4();

		result.m[0][0] = scale.x;
		result.m[1][1] = scale.y;
		result.m[2][2] = scale.z;

		return result;
	}

	Matrix4x4 MakeTranslateMatrix(const Vector3& translation) {
		Matrix4x4 result = Identity4x4();

		result.m[3][0] = translation.x;
		result.m[3][1] = translation.y;
		result.m[3][2] = translation.z;

		return result;
	}

	Matrix4x4 MakeRotateXMatrix(float rad) {
		Matrix4x4 result = Identity4x4();

		const float sin = std::sin(rad);
		const float cos = std::cos(rad);

		result.m[1][1] = cos;
		result.m[1][2] = sin;
		result.m[2][1] = -sin;
		result.m[2][2] = cos;

		return result;
	}

	Matrix4x4 MakeRotateYMatrix(float rad) {
		Matrix4x4 result = Identity4x4();

		const float sin = std::sin(rad);
		const float cos = std::cos(rad);

		result.m[0][0] = cos;
		result.m[0][2] = -sin;
		result.m[2][0] = sin;
		result.m[2][2] = cos;

		return result;
	}

	Matrix4x4 MakeRotateZMatrix(float rad) {
		Matrix4x4 result = Identity4x4();

		const float sin = std::sin(rad);
		const float cos = std::cos(rad);

		result.m[0][0] = cos;
		result.m[0][1] = sin;
		result.m[1][0] = -sin;
		result.m[1][1] = cos;

		return result;
	}

	Matrix4x4 MakeRotateMatrix(const Quaternion& rotation) {
		Matrix4x4 result = Identity4x4();

		const float x = rotation.x;
		const float y = rotation.y;
		const float z = rotation.z;
		const float w = rotation.w;

		result.m[0][0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
		result.m[0][1] = 2.0f * x * y + 2.0f * w * z;
		result.m[0][2] = 2.0f * x * z - 2.0f * w * y;

		result.m[1][0] = 2.0f * x * y - 2.0f * w * z;
		result.m[1][1] = 1.0f - 2.0f * x * x - 2.0f * z * z;
		result.m[1][2] = 2.0f * y * z + 2.0f * w * x;

		result.m[2][0] = 2.0f * x * z + 2.0f * w * y;
		result.m[2][1] = 2.0f * y * z - 2.0f * w * x;
		result.m[2][2] = 1.0f - 2.0f * x * x - 2.0f * y * y;

		return result;
	}

	Matrix4x4 MakeAffineMatrix(
		const Vector3& scale,
		const Quaternion& rotation,
		const Vector3& translation) {

		const Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
		const Matrix4x4 rotateMatrix = MakeRotateMatrix(rotation);
		const Matrix4x4 translateMatrix = MakeTranslateMatrix(translation);

		return Multiply(Multiply(scaleMatrix, rotateMatrix), translateMatrix);
	}
}