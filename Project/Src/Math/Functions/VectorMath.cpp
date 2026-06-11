#include "LGF/Math/Functions/VectorMath.h"

#include "LGF/Math/MathConstants.h"

#include <cmath>

namespace LGF::Math {

	//====================================================================
	// 長さ
	//====================================================================

	float Length(const Vector2& v) {
		return std::sqrt(v.x * v.x + v.y * v.y);
	}

	float Length(const Vector3& v) {
		return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	float Length(const Vector4& v) {
		return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
	}

	//====================================================================
	// 長さの二乗
	//====================================================================

	float LengthSq(const Vector2& v) {
		return v.x * v.x + v.y * v.y;
	}

	float LengthSq(const Vector3& v) {
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}

	float LengthSq(const Vector4& v) {
		return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
	}

	//====================================================================
	// 正規化
	//====================================================================

	Vector2 Normalize(const Vector2& v) {

		const float length = Length(v);

		if (length < EPSILON) {
			return {};
		}

		return v / length;
	}

	Vector3 Normalize(const Vector3& v) {

		const float length = Length(v);

		if (length < EPSILON) {
			return {};
		}

		return v / length;
	}

	Vector4 Normalize(const Vector4& v) {

		const float length = Length(v);

		if (length < EPSILON) {
			return {};
		}

		return v / length;
	}

	//====================================================================
	// 内積
	//====================================================================

	float Dot(const Vector2& v1, const Vector2& v2) {
		return v1.x * v2.x +
			v1.y * v2.y;
	}

	float Dot(const Vector3& v1, const Vector3& v2) {
		return v1.x * v2.x +
			v1.y * v2.y +
			v1.z * v2.z;
	}

	float Dot(const Vector4& v1, const Vector4& v2) {
		return v1.x * v2.x +
			v1.y * v2.y +
			v1.z * v2.z +
			v1.w * v2.w;
	}

	//====================================================================
	// 外積
	//====================================================================

	Vector3 Cross(const Vector3& v1, const Vector3& v2) {
		return {
			v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x
		};
	}

	//====================================================================
	// 距離
	//====================================================================

	float Distance(const Vector2& v1, const Vector2& v2) {
		return Length(v2 - v1);
	}

	float Distance(const Vector3& v1, const Vector3& v2) {
		return Length(v2 - v1);
	}

	float Distance(const Vector4& v1, const Vector4& v2) {
		return Length(v2 - v1);
	}

	//====================================================================
	// 距離の二乗
	//====================================================================

	float DistanceSq(const Vector2& v1, const Vector2& v2) {
		return LengthSq(v2 - v1);
	}

	float DistanceSq(const Vector3& v1, const Vector3& v2) {
		return LengthSq(v2 - v1);
	}

	float DistanceSq(const Vector4& v1, const Vector4& v2) {
		return LengthSq(v2 - v1);
	}

}