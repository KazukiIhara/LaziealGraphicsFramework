#pragma once

namespace LGF {

	/// <summary>
	/// 二次元ベクトル
	/// </summary>
	struct Vector2 final {
		float x = 0.0f;
		float y = 0.0f;

		Vector2& operator+=(const Vector2& v);
		Vector2& operator-=(const Vector2& v);
		Vector2& operator*=(const Vector2& v);
		Vector2& operator/=(const Vector2& v);

		Vector2& operator*=(float s);
		Vector2& operator/=(float s);
	};

	Vector2 operator-(const Vector2& v);
	Vector2 operator+(const Vector2& v);

	Vector2 operator+(const Vector2& v1, const Vector2& v2);
	Vector2 operator-(const Vector2& v1, const Vector2& v2);
	Vector2 operator*(const Vector2& v1, const Vector2& v2);
	Vector2 operator/(const Vector2& v1, const Vector2& v2);

	Vector2 operator*(const Vector2& v, float s);
	Vector2 operator*(float s, const Vector2& v);
	Vector2 operator/(const Vector2& v, float s);

	bool operator==(const Vector2& v1, const Vector2& v2);
	bool operator!=(const Vector2& v1, const Vector2& v2);
}
