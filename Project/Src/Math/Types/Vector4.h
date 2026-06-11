#pragma once

namespace LGF {

	/// <summary>
	/// 四次元ベクトル
	/// </summary>
	struct Vector4 final {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float w = 0.0f;

		Vector4& operator+=(const Vector4& v);
		Vector4& operator-=(const Vector4& v);
		Vector4& operator*=(const Vector4& v);
		Vector4& operator/=(const Vector4& v);

		Vector4& operator*=(float s);
		Vector4& operator/=(float s);
	};

	Vector4 operator-(const Vector4& v);
	Vector4 operator+(const Vector4& v);

	Vector4 operator+(const Vector4& v1, const Vector4& v2);
	Vector4 operator-(const Vector4& v1, const Vector4& v2);
	Vector4 operator*(const Vector4& v1, const Vector4& v2);
	Vector4 operator/(const Vector4& v1, const Vector4& v2);

	Vector4 operator*(const Vector4& v, float s);
	Vector4 operator*(float s, const Vector4& v);
	Vector4 operator/(const Vector4& v, float s);

	bool operator==(const Vector4& v1, const Vector4& v2);
	bool operator!=(const Vector4& v1, const Vector4& v2);
}
