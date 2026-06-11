#include "Vector3.h"

namespace LGF {

	Vector3& Vector3::operator+=(const Vector3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3& Vector3::operator-=(const Vector3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3& Vector3::operator*=(const Vector3& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	Vector3& Vector3::operator/=(const Vector3& v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	Vector3& Vector3::operator*=(float s) {
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	Vector3& Vector3::operator/=(float s) {
		x /= s;
		y /= s;
		z /= s;
		return *this;
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

	Vector3 operator*(const Vector3& v, float s) {
		return { v.x * s, v.y * s, v.z * s };
	}

	Vector3 operator*(float s, const Vector3& v) {
		return { s * v.x, s * v.y, s * v.z };
	}

	Vector3 operator/(const Vector3& v, float s) {
		return { v.x / s, v.y / s, v.z / s };
	}

	bool operator==(const Vector3& v1, const Vector3& v2) {
		return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
	}

	bool operator!=(const Vector3& v1, const Vector3& v2) {
		return !(v1 == v2);
	}
}
