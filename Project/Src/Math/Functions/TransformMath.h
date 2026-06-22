#pragma once

#include "LGF/Math/Vector3.h"
#include "LGF/Math/Quaternion.h"
#include "LGF/Math/Matrix4x4.h"

namespace LGF::Math {

	// スケール
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	// 平行移動
	Matrix4x4 MakeTranslateMatrix(const Vector3& translation);

	// 単軸回転
	Matrix4x4 MakeRotateXMatrix(float rad);
	Matrix4x4 MakeRotateYMatrix(float rad);
	Matrix4x4 MakeRotateZMatrix(float rad);

	// クォータニオン回転
	Matrix4x4 MakeRotateMatrix(const Quaternion& rotation);

	// アフィン変換
	Matrix4x4 MakeAffineMatrix(
		const Vector3& scale,
		const Quaternion& rotation,
		const Vector3& translation);

}