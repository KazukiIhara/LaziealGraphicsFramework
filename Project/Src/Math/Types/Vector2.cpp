#include "Vector2.h"

namespace LGF {

	Vector2& Vector2::operator+=(const Vector2& v) {
		x += v.x;
		y += v.y;
		return *this;
	}

	Vector2& Vector2::operator-=(const Vector2& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}

	Vector2& Vector2::operator*=(const Vector2& v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}

	Vector2& Vector2::operator/=(const Vector2& v) {
		x /= v.x;
		y /= v.y;
		return *this;
	}

	Vector2& Vector2::operator*=(float s) {
		x *= s;
		y *= s;
		return *this;
	}

	Vector2& Vector2::operator/=(float s) {
		x /= s;
		y /= s;
		return *this;
	}

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

	Vector2 operator*(const Vector2& v, float s) {
		return { v.x * s, v.y * s };
	}

	Vector2 operator*(float s, const Vector2& v) {
		return { s * v.x, s * v.y };
	}

	Vector2 operator/(const Vector2& v, float s) {
		return { v.x / s, v.y / s };
	}

	bool operator==(const Vector2& v1, const Vector2& v2) {
		return v1.x == v2.x && v1.y == v2.y;
	}

	bool operator!=(const Vector2& v1, const Vector2& v2) {
		return !(v1 == v2);
	}
}
