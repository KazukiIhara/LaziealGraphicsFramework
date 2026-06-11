#include "LGF/Math/MathUtilities.h"

#include <cmath>
#include <cstdint>

namespace LGF::Math {

	bool NearEqual(float v1, float v2, float epsilon) {
		return std::fabs(v1 - v2) <= epsilon;
	}

	bool NearEqual(const Vector2& v1, const Vector2& v2, float epsilon) {
		return
			NearEqual(v1.x, v2.x, epsilon) &&
			NearEqual(v1.y, v2.y, epsilon);
	}

	bool NearEqual(const Vector3& v1, const Vector3& v2, float epsilon) {
		return
			NearEqual(v1.x, v2.x, epsilon) &&
			NearEqual(v1.y, v2.y, epsilon) &&
			NearEqual(v1.z, v2.z, epsilon);
	}

	bool NearEqual(const Vector4& v1, const Vector4& v2, float epsilon) {
		return
			NearEqual(v1.x, v2.x, epsilon) &&
			NearEqual(v1.y, v2.y, epsilon) &&
			NearEqual(v1.z, v2.z, epsilon) &&
			NearEqual(v1.w, v2.w, epsilon);
	}

	bool NearEqual(const Matrix4x4& m1, const Matrix4x4& m2, float epsilon) {
		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				if (!NearEqual(m1.m[y][x], m2.m[y][x], epsilon)) {
					return false;
				}
			}
		}

		return true;
	}

	bool NearEqual(const Quaternion& q1, const Quaternion& q2, float epsilon) {
		if (NearEqual(q1.x, q2.x, epsilon) &&
			NearEqual(q1.y, q2.y, epsilon) &&
			NearEqual(q1.z, q2.z, epsilon) &&
			NearEqual(q1.w, q2.w, epsilon)) {
			return true;
		}

		if (NearEqual(q1.x, -q2.x, epsilon) &&
			NearEqual(q1.y, -q2.y, epsilon) &&
			NearEqual(q1.z, -q2.z, epsilon) &&
			NearEqual(q1.w, -q2.w, epsilon)) {
			return true;
		}

		return false;
	}


}