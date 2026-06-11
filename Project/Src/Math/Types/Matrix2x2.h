#pragma once

namespace LGF {

	/// <summary>
	/// 2x2行列
	/// </summary>
	struct Matrix2x2 final {
		float m[2][2] = {};

		Matrix2x2& operator+=(const Matrix2x2& mat);
		Matrix2x2& operator-=(const Matrix2x2& mat);
		Matrix2x2& operator*=(float s);
		Matrix2x2& operator/=(float s);
	};

	Matrix2x2 operator-(const Matrix2x2& mat);
	Matrix2x2 operator+(const Matrix2x2& mat);

	Matrix2x2 operator+(const Matrix2x2& lhs, const Matrix2x2& rhs);
	Matrix2x2 operator-(const Matrix2x2& lhs, const Matrix2x2& rhs);
	Matrix2x2 operator*(const Matrix2x2& mat, float s);
	Matrix2x2 operator*(float s, const Matrix2x2& mat);
	Matrix2x2 operator/(const Matrix2x2& mat, float s);

	bool operator==(const Matrix2x2& lhs, const Matrix2x2& rhs);
	bool operator!=(const Matrix2x2& lhs, const Matrix2x2& rhs);

}