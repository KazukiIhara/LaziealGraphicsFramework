#pragma once

#include "Math/Types/Vector2.h"
#include "Math/Types/Vector3.h"

#include "Math/Types/Matrix2x2.h"
#include "Math/Types/Matrix3x3.h"
#include "Math/Types/Matrix4x4.h"

namespace LGF::Math {

	// 単位行列
	Matrix2x2 Identity2x2();
	Matrix3x3 Identity3x3();
	Matrix4x4 Identity4x4();

	// 転置行列
	Matrix2x2 Transpose(const Matrix2x2& m);
	Matrix3x3 Transpose(const Matrix3x3& m);
	Matrix4x4 Transpose(const Matrix4x4& m);

	// 行列積
	Matrix2x2 Multiply(const Matrix2x2& m1, const Matrix2x2& m2);
	Matrix3x3 Multiply(const Matrix3x3& m1, const Matrix3x3& m2);
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	// 行列式
	float Determinant(const Matrix2x2& m);
	float Determinant(const Matrix3x3& m);
	float Determinant(const Matrix4x4& m);

	// 逆行列
	Matrix2x2 Inverse(const Matrix2x2& m);
	Matrix3x3 Inverse(const Matrix3x3& m);
	Matrix4x4 Inverse(const Matrix4x4& m);

	// ベクトル変換
	Vector2 Transform(const Vector2& v, const Matrix3x3& m);
	Vector3 Transform(const Vector3& v, const Matrix4x4& m);

	// 方向ベクトル変換
	Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);
}