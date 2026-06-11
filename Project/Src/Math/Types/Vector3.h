#pragma once

namespace LGF {

	/// <summary>
	/// 三次元ベクトル
	/// </summary>
	struct Vector3 final {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		Vector3& operator+=(const Vector3& v);
		Vector3& operator-=(const Vector3& v);
		Vector3& operator*=(const Vector3& v);
		Vector3& operator/=(const Vector3& v);

		Vector3& operator*=(float s);
		Vector3& operator/=(float s);
	};

	Vector3 operator-(const Vector3& v);
	Vector3 operator+(const Vector3& v);

	Vector3 operator+(const Vector3& v1, const Vector3& v2);
	Vector3 operator-(const Vector3& v1, const Vector3& v2);
	Vector3 operator*(const Vector3& v1, const Vector3& v2);
	Vector3 operator/(const Vector3& v1, const Vector3& v2);

	Vector3 operator*(const Vector3& v, float s);
	Vector3 operator*(float s, const Vector3& v);
	Vector3 operator/(const Vector3& v, float s);

	bool operator==(const Vector3& v1, const Vector3& v2);
	bool operator!=(const Vector3& v1, const Vector3& v2);
}
