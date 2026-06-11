#pragma once

#include "MathTypes.h"


// 左手座標系
// 行ベクトル
// Y → X → Z の掛け順を基準とする

namespace LGF::Math {

	inline constexpr float EPSILON = 1.0e-6f;

	bool NearEqual(float v1, float v2, float epsilon = EPSILON);
	bool NearEqual(const Vector2& v1, const Vector2& v2, float epsilon = EPSILON);
	bool NearEqual(const Vector3& v1, const Vector3& v2, float epsilon = EPSILON);
	bool NearEqual(const Vector4& v1, const Vector4& v2, float epsilon = EPSILON);
	bool NearEqual(const Matrix4x4& m1, const Matrix4x4& m2, float epsilon = EPSILON);
	bool NearEqual(const Quaternion& q1, const Quaternion& q2, float epsilon = EPSILON);

}