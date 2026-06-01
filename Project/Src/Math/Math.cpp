#include "LGF/Math/Math.h"

#include <cmath>
#include <cstdint>

namespace LGF::Math {

	Vector2 operator-(const Vector2& v) {
		return { -v.x, -v.y };
	}

	Vector2 operator+(const Vector2& v) {
		return v;
	}

	Vector2 operator+(const Vector2& v1, const Vector2& v2) {
		return { v1.x + v2.x, v1.y + v2.y };
	}

	Vector2 operator-(const Vector2& v1, const Vector2& v2) {
		return { v1.x - v2.x, v1.y - v2.y };
	}

	Vector2 operator*(const Vector2& v1, const Vector2& v2) {
		return { v1.x * v2.x, v1.y * v2.y };
	}

	Vector2 operator/(const Vector2& v1, const Vector2& v2) {
		return { v1.x / v2.x, v1.y / v2.y };
	}

	Vector2 operator/(const Vector2& v, float s) {
		return { v.x / s, v.y / s };
	}

	Vector2 operator*(float s, const Vector2& v) {
		return { s * v.x, s * v.y };
	}

	Vector2 operator*(const Vector2& v, float s) {
		return { v.x * s, v.y * s };
	}

	bool operator!=(const Vector2& v1, const Vector2& v2) {
		return !(v1 == v2);
	}

	bool operator==(const Vector2& v1, const Vector2& v2) {
		return v1.x == v2.x && v1.y == v2.y;
	}

	Vector3 operator-(const Vector3& v) {
		return { -v.x, -v.y, -v.z };
	}

	Vector3 operator+(const Vector3& v) {
		return v;
	}

	Vector3 operator+(const Vector3& v1, const Vector3& v2) {
		return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	}

	Vector3 operator-(const Vector3& v1, const Vector3& v2) {
		return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	}

	Vector3 operator*(const Vector3& v1, const Vector3& v2) {
		return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
	}

	Vector3 operator/(const Vector3& v1, const Vector3& v2) {
		return { v1.x / v2.x, v1.y / v2.y, v1.z / v2.z };
	}

	Vector3 operator*(float s, const Vector3& v) {
		return { s * v.x, s * v.y, s * v.z };
	}

	Vector3 operator*(const Vector3& v, float s) {
		return { v.x * s, v.y * s, v.z * s };
	}

	Vector3 operator/(const Vector3& v, float s) {
		return { v.x / s, v.y / s, v.z / s };
	}

	bool operator!=(const Vector3& v1, const Vector3& v2) {
		return !(v1 == v2);
	}

	bool operator==(const Vector3& v1, const Vector3& v2) {
		return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
	}

	Vector4 operator-(const Vector4& v) {
		return { -v.x, -v.y, -v.z, -v.w };
	}

	Vector4 operator+(const Vector4& v) {
		return v;
	}

	Vector4 operator+(const Vector4& v1, const Vector4& v2) {
		return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
	}

	Vector4 operator-(const Vector4& v1, const Vector4& v2) {
		return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
	}

	Vector4 operator*(const Vector4& v1, const Vector4& v2) {
		return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w };
	}

	Vector4 operator/(const Vector4& v1, const Vector4& v2) {
		return { v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w };
	}

	Vector4 operator*(float s, const Vector4& v) {
		return { s * v.x, s * v.y, s * v.z, s * v.w };
	}

	Vector4 operator*(const Vector4& v, float s) {
		return { v.x * s, v.y * s, v.z * s, v.w * s };
	}

	Vector4 operator/(const Vector4& v, float s) {
		return { v.x / s, v.y / s, v.z / s, v.w / s };
	}

	bool operator!=(const Vector4& v1, const Vector4& v2) {
		return !(v1 == v2);
	}

	bool operator==(const Vector4& v1, const Vector4& v2) {
		return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
	}

	Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2) {
		Matrix4x4 result{};

		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				result.m[y][x] = m1.m[y][x] + m2.m[y][x];
			}
		}

		return result;
	}

	Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2) {
		Matrix4x4 result{};

		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				result.m[y][x] = m1.m[y][x] - m2.m[y][x];
			}
		}

		return result;
	}

	Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) {
		Matrix4x4 result{};

		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				result.m[y][x] =
					m1.m[y][0] * m2.m[0][x] +
					m1.m[y][1] * m2.m[1][x] +
					m1.m[y][2] * m2.m[2][x] +
					m1.m[y][3] * m2.m[3][x];
			}
		}

		return result;
	}

	bool operator!=(const Matrix4x4& m1, const Matrix4x4& m2) {
		return !(m1 == m2);
	}

	bool operator==(const Matrix4x4& m1, const Matrix4x4& m2) {
		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				if (m1.m[y][x] != m2.m[y][x]) {
					return false;
				}
			}
		}

		return true;
	}

	Quaternion operator+(const Quaternion& q1, const Quaternion& q2) {
		return {
			q1.x + q2.x,
			q1.y + q2.y,
			q1.z + q2.z,
			q1.w + q2.w
		};
	}

	Quaternion operator-(const Quaternion& q1, const Quaternion& q2) {
		return {
			q1.x - q2.x,
			q1.y - q2.y,
			q1.z - q2.z,
			q1.w - q2.w
		};
	}

	Quaternion operator*(const Quaternion& q1, const Quaternion& q2) {
		return {
			q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
			q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
			q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
			q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
		};
	}

	bool operator!=(const Quaternion& q1, const Quaternion& q2) {
		return !(q1 == q2);
	}

	bool operator==(const Quaternion& q1, const Quaternion& q2) {
		return q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w;
	}

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