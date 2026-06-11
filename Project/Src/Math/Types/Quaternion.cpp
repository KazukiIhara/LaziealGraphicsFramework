#include "Quaternion.h"

namespace LGF {

	Quaternion& Quaternion::operator+=(const Quaternion& q) {
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;
		return *this;
	}

	Quaternion& Quaternion::operator-=(const Quaternion& q) {
		x -= q.x;
		y -= q.y;
		z -= q.z;
		w -= q.w;
		return *this;
	}

	Quaternion& Quaternion::operator*=(const Quaternion& q) {
		*this = *this * q;
		return *this;
	}

	Quaternion& Quaternion::operator*=(float s) {
		x *= s;
		y *= s;
		z *= s;
		w *= s;
		return *this;
	}

	Quaternion& Quaternion::operator/=(float s) {
		x /= s;
		y /= s;
		z /= s;
		w /= s;
		return *this;
	}

	Quaternion operator-(const Quaternion& q) {
		return { -q.x, -q.y, -q.z, -q.w };
	}

	Quaternion operator+(const Quaternion& q) {
		return q;
	}

	Quaternion operator+(const Quaternion& q1, const Quaternion& q2) {
		return { q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w };
	}

	Quaternion operator-(const Quaternion& q1, const Quaternion& q2) {
		return { q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w };
	}

	Quaternion operator*(const Quaternion& q1, const Quaternion& q2) {
		return {
			q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
			q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
			q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
			q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
		};
	}

	Quaternion operator*(const Quaternion& q, float s) {
		return { q.x * s, q.y * s, q.z * s, q.w * s };
	}

	Quaternion operator*(float s, const Quaternion& q) {
		return { s * q.x, s * q.y, s * q.z, s * q.w };
	}

	Quaternion operator/(const Quaternion& q, float s) {
		return { q.x / s, q.y / s, q.z / s, q.w / s };
	}

	bool operator==(const Quaternion& q1, const Quaternion& q2) {
		return q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w;
	}

	bool operator!=(const Quaternion& q1, const Quaternion& q2) {
		return !(q1 == q2);
	}
}
