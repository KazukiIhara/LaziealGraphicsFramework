#include "LGF/Math/Vector4.h"

namespace LGF {

	Vector4& Vector4::operator+=(const Vector4& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	Vector4& Vector4::operator-=(const Vector4& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	Vector4& Vector4::operator*=(const Vector4& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	Vector4& Vector4::operator/=(const Vector4& v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	Vector4& Vector4::operator*=(float s) {
		x *= s;
		y *= s;
		z *= s;
		w *= s;
		return *this;
	}

	Vector4& Vector4::operator/=(float s) {
		x /= s;
		y /= s;
		z /= s;
		w /= s;
		return *this;
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

	Vector4 operator*(const Vector4& v, float s) {
		return { v.x * s, v.y * s, v.z * s, v.w * s };
	}

	Vector4 operator*(float s, const Vector4& v) {
		return { s * v.x, s * v.y, s * v.z, s * v.w };
	}

	Vector4 operator/(const Vector4& v, float s) {
		return { v.x / s, v.y / s, v.z / s, v.w / s };
	}

	bool operator==(const Vector4& v1, const Vector4& v2) {
		return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
	}

	bool operator!=(const Vector4& v1, const Vector4& v2) {
		return !(v1 == v2);
	}
}
