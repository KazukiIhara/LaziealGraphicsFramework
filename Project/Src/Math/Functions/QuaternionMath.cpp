#include "LGF/Math/Functions/QuaternionMath.h"

#include "LGF/Math/MathConstants.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace LGF::Math {

	float Length(const Quaternion& q) {
		return std::sqrt(LengthSq(q));
	}

	float LengthSq(const Quaternion& q) {
		return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	}

	Quaternion Normalize(const Quaternion& q) {
		const float length = Length(q);

		if (length <= EPSILON) {
			return {};
		}

		return q / length;
	}

	Quaternion Conjugate(const Quaternion& q) {
		return { -q.x, -q.y, -q.z, q.w };
	}

	Quaternion Inverse(const Quaternion& q) {
		const float lengthSq = LengthSq(q);

		assert(lengthSq > EPSILON);

		if (lengthSq <= EPSILON) {
			return {};
		}

		return Conjugate(q) / lengthSq;
	}

	float Dot(const Quaternion& q1, const Quaternion& q2) {
		return
			q1.x * q2.x +
			q1.y * q2.y +
			q1.z * q2.z +
			q1.w * q2.w;
	}

	Quaternion MakeRotateAxisAngle(
		const Vector3& axis,
		float rad) {

		const float axisLength =
			std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);

		assert(axisLength > EPSILON);

		if (axisLength <= EPSILON) {
			return {};
		}

		const float invAxisLength = 1.0f / axisLength;

		const Vector3 normalizedAxis = {
			axis.x * invAxisLength,
			axis.y * invAxisLength,
			axis.z * invAxisLength
		};

		const float halfRad = rad * 0.5f;
		const float sinHalf = std::sin(halfRad);
		const float cosHalf = std::cos(halfRad);

		return {
			normalizedAxis.x * sinHalf,
			normalizedAxis.y * sinHalf,
			normalizedAxis.z * sinHalf,
			cosHalf
		};
	}

	Quaternion EulerToQuaternionXYZ(
		const Vector3& euler) {

		const Quaternion rotateX =
			MakeRotateAxisAngle({ 1.0f, 0.0f, 0.0f }, euler.x);

		const Quaternion rotateY =
			MakeRotateAxisAngle({ 0.0f, 1.0f, 0.0f }, euler.y);

		const Quaternion rotateZ =
			MakeRotateAxisAngle({ 0.0f, 0.0f, 1.0f }, euler.z);

		return Normalize(rotateX * rotateY * rotateZ);
	}

	Quaternion EulerToQuaternionYXZ(
		const Vector3& euler) {

		const Quaternion rotateY =
			MakeRotateAxisAngle({ 0.0f, 1.0f, 0.0f }, euler.y);

		const Quaternion rotateX =
			MakeRotateAxisAngle({ 1.0f, 0.0f, 0.0f }, euler.x);

		const Quaternion rotateZ =
			MakeRotateAxisAngle({ 0.0f, 0.0f, 1.0f }, euler.z);

		return Normalize(rotateY * rotateX * rotateZ);
	}

	Quaternion Slerp(
		const Quaternion& q1,
		const Quaternion& q2,
		float t) {

		Quaternion from = Normalize(q1);
		Quaternion to = Normalize(q2);

		float dot = Dot(from, to);

		if (dot < 0.0f) {
			to = -to;
			dot = -dot;
		}

		dot = std::clamp(dot, -1.0f, 1.0f);

		if (dot > 1.0f - EPSILON) {
			return Normalize(from + (to - from) * t);
		}

		const float theta = std::acos(dot);
		const float sinTheta = std::sin(theta);

		assert(std::fabs(sinTheta) > EPSILON);

		if (std::fabs(sinTheta) <= EPSILON) {
			return from;
		}

		const float weightFrom = std::sin((1.0f - t) * theta) / sinTheta;
		const float weightTo = std::sin(t * theta) / sinTheta;

		return Normalize(from * weightFrom + to * weightTo);
	}
}