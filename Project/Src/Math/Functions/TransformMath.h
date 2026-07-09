#pragma once

#include "LGF/Math/Vector2.h"
#include "LGF/Math/Vector3.h"
#include "LGF/Math/Quaternion.h"
#include "LGF/Math/Matrix3x3.h"
#include "LGF/Math/Matrix4x4.h"

namespace LGF::Math {

	// スケール
	Matrix3x3 MakeScaleMatrix(const Vector2& scale);
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	// 平行移動
	Matrix3x3 MakeTranslateMatrix(const Vector2& translation);
	Matrix4x4 MakeTranslateMatrix(const Vector3& translation);

	// 回転
	Matrix3x3 MakeRotateMatrix(float rad);

	// 単軸回転
	Matrix4x4 MakeRotateXMatrix(float rad);
	Matrix4x4 MakeRotateYMatrix(float rad);
	Matrix4x4 MakeRotateZMatrix(float rad);

	// クォータニオン回転
	Matrix4x4 MakeRotateMatrix(const Quaternion& rotation);

	// アフィン変換
	Matrix3x3 MakeAffineMatrix(
		const Vector2& scale,
		float rotation,
		const Vector2& translation);

	// アフィン変換
	Matrix4x4 MakeAffineMatrix(
		const Vector3& scale,
		const Quaternion& rotation,
		const Vector3& translation);

}