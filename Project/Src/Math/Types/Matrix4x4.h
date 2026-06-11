#pragma once

namespace LGF {

	/// <summary>
	/// 4x4行列
	/// </summary>
	struct Matrix4x4 final {
		float m[4][4] = {};

		Matrix4x4& operator+=(const Matrix4x4& m);
		Matrix4x4& operator-=(const Matrix4x4& m);
		Matrix4x4& operator*=(float s);
		Matrix4x4& operator/=(float s);
	};

	Matrix4x4 operator-(const Matrix4x4& m);
	Matrix4x4 operator+(const Matrix4x4& m);

	Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2);
	Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2);
	Matrix4x4 operator*(const Matrix4x4& m, float s);
	Matrix4x4 operator*(float s, const Matrix4x4& m);
	Matrix4x4 operator/(const Matrix4x4& m, float s);

	bool operator==(const Matrix4x4& m1, const Matrix4x4& m2);
	bool operator!=(const Matrix4x4& m1, const Matrix4x4& m2);
}
