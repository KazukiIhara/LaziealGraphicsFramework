#pragma once

#include "LGF/Math/Quaternion.h"
#include "LGF/Math/Vector3.h"
#include "LGF/Math/Matrix4x4.h"

namespace LGF::Math {

	// 長さ
	float Length(const Quaternion& q);

	// 長さの二乗
	float LengthSq(const Quaternion& q);

	// 正規化
	Quaternion Normalize(const Quaternion& q);

	// 共役
	Quaternion Conjugate(const Quaternion& q);

	// 逆
	Quaternion Inverse(const Quaternion& q);

	// 内積
	float Dot(const Quaternion& q1, const Quaternion& q2);

	// 軸角
	Quaternion MakeRotateAxisAngle(const Vector3& axis, float rad);

	// オイラー角からクオータニオンを作成(Y→X→Z)
	Quaternion MakeQuaternion(const Vector3& euler);

	// 補間
	Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);

}