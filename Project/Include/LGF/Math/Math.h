#pragma once

#include "MathTypes.h"

// 左手座標系
// 行ベクトル
// Y → X → Z の掛け順を基準とする

namespace LGF::Math {

	inline constexpr float EPSILON = 1.0e-6f;


	Vector2 operator-(const Vector2& v);
	Vector2 operator+(const Vector2& v);
	Vector2 operator+(const Vector2& v1, const Vector2& v2);
	Vector2 operator-(const Vector2& v1, const Vector2& v2);
	Vector2 operator*(const Vector2& v1, const Vector2& v2);
	Vector2 operator/(const Vector2& v1, const Vector2& v2);
	Vector2 operator/(const Vector2& v, float s);
	Vector2 operator*(float s, const Vector2& v);
	Vector2 operator*(const Vector2& v, float s);
	bool operator!=(const Vector2& v1, const Vector2& v2);
	bool operator==(const Vector2& v1, const Vector2& v2);

	Vector3 operator-(const Vector3& v);
	Vector3 operator+(const Vector3& v);
	Vector3 operator+(const Vector3& v1, const Vector3& v2);
	Vector3 operator-(const Vector3& v1, const Vector3& v2);
	Vector3 operator*(const Vector3& v1, const Vector3& v2);
	Vector3 operator/(const Vector3& v1, const Vector3& v2);
	Vector3 operator*(float s, const Vector3& v);
	Vector3 operator*(const Vector3& v, float s);
	Vector3 operator/(const Vector3& v, float s);
	bool operator!=(const Vector3& v1, const Vector3& v2);
	bool operator==(const Vector3& v1, const Vector3& v2);

	Vector4 operator-(const Vector4& v);
	Vector4 operator+(const Vector4& v);
	Vector4 operator+(const Vector4& v1, const Vector4& v2);
	Vector4 operator-(const Vector4& v1, const Vector4& v2);
	Vector4 operator*(const Vector4& v1, const Vector4& v2);
	Vector4 operator/(const Vector4& v1, const Vector4& v2);
	Vector4 operator*(float s, const Vector4& v);
	Vector4 operator*(const Vector4& v, float s);
	Vector4 operator/(const Vector4& v, float s);
	bool operator!=(const Vector4& v1, const Vector4& v2);
	bool operator==(const Vector4& v1, const Vector4& v2);

	Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2);
	Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2);
	Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);
	bool operator!=(const Matrix4x4& m1, const Matrix4x4& m2);
	bool operator==(const Matrix4x4& m1, const Matrix4x4& m2);

	Quaternion operator+(const Quaternion& q1, const Quaternion& q2);
	Quaternion operator-(const Quaternion& q1, const Quaternion& q2);
	Quaternion operator*(const Quaternion& q1, const Quaternion& q2);
	bool operator!=(const Quaternion& q1, const Quaternion& q2);
	bool operator==(const Quaternion& q1, const Quaternion& q2);


	bool NearEqual(float v1, float v2, float epsilon = EPSILON);
	bool NearEqual(const Vector2& v1, const Vector2& v2, float epsilon = EPSILON);
	bool NearEqual(const Vector3& v1, const Vector3& v2, float epsilon = EPSILON);
	bool NearEqual(const Vector4& v1, const Vector4& v2, float epsilon = EPSILON);
	bool NearEqual(const Matrix4x4& m1, const Matrix4x4& m2, float epsilon = EPSILON);
	bool NearEqual(const Quaternion& q1, const Quaternion& q2, float epsilon = EPSILON);

}