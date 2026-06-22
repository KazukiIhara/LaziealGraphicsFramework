#pragma once

#include "LGF/Math/Vector2.h"
#include "LGF/Math/Vector3.h"
#include "LGF/Math/Vector4.h"
 
namespace LGF::Math {

	// 長さ
	float Length(const Vector2& v);
	float Length(const Vector3& v);
	float Length(const Vector4& v);

	// 長さの二乗
	float LengthSq(const Vector2& v);
	float LengthSq(const Vector3& v);
	float LengthSq(const Vector4& v);

	// 正規化
	Vector2 Normalize(const Vector2& v);
	Vector3 Normalize(const Vector3& v);
	Vector4 Normalize(const Vector4& v);

	// 内積
	float Dot(const Vector2& v1, const Vector2& v2);
	float Dot(const Vector3& v1, const Vector3& v2);
	float Dot(const Vector4& v1, const Vector4& v2);

	// 外積
	Vector3 Cross(const Vector3& v1, const Vector3& v2);

	// 距離
	float Distance(const Vector2& v1, const Vector2& v2);
	float Distance(const Vector3& v1, const Vector3& v2);
	float Distance(const Vector4& v1, const Vector4& v2);

	// 距離の二乗
	float DistanceSq(const Vector2& v1, const Vector2& v2);
	float DistanceSq(const Vector3& v1, const Vector3& v2);
	float DistanceSq(const Vector4& v1, const Vector4& v2);

}