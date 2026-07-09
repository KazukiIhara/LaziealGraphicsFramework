#pragma once

namespace LGF {

	/// <summary>
	/// 3x3行列
	/// </summary>
	struct Matrix3x3 final {
		float m[3][3] = {};

		Matrix3x3& operator+=(const Matrix3x3& mat);
		Matrix3x3& operator-=(const Matrix3x3& mat);
		Matrix3x3& operator*=(float s);
		Matrix3x3& operator/=(float s);
	};

	Matrix3x3 operator-(const Matrix3x3& m);
	Matrix3x3 operator+(const Matrix3x3& m);

	Matrix3x3 operator+(const Matrix3x3& m1, const Matrix3x3& m2);
	Matrix3x3 operator-(const Matrix3x3& m1, const Matrix3x3& m2);
	Matrix3x3 operator*(const Matrix3x3& m1, const Matrix3x3& m2);
	Matrix3x3 operator*(const Matrix3x3& m, float s);
	Matrix3x3 operator*(float s, const Matrix3x3& m);
	Matrix3x3 operator/(const Matrix3x3& m, float s);

	bool operator==(const Matrix3x3& m1, const Matrix3x3& m2);
	bool operator!=(const Matrix3x3& m1, const Matrix3x3& m2);
}
