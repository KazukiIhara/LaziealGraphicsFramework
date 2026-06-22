#pragma once

namespace LGF {

	/// <summary>
	/// クォータニオン
	/// </summary>
	struct Quaternion final {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float w = 1.0f;

		Quaternion& operator+=(const Quaternion& q);
		Quaternion& operator-=(const Quaternion& q);
		Quaternion& operator*=(const Quaternion& q);

		Quaternion& operator*=(float s);
		Quaternion& operator/=(float s);
	};

	Quaternion operator-(const Quaternion& q);
	Quaternion operator+(const Quaternion& q);

	Quaternion operator+(const Quaternion& q1, const Quaternion& q2);
	Quaternion operator-(const Quaternion& q1, const Quaternion& q2);
	Quaternion operator*(const Quaternion& q1, const Quaternion& q2);

	Quaternion operator*(const Quaternion& q, float s);
	Quaternion operator*(float s, const Quaternion& q);
	Quaternion operator/(const Quaternion& q, float s);

	bool operator==(const Quaternion& q1, const Quaternion& q2);
	bool operator!=(const Quaternion& q1, const Quaternion& q2);
}
