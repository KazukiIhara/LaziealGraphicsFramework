#pragma once

#include "LGF/Math/Matrix4x4.h"

namespace LGF::Math {

	// 透視投影
	Matrix4x4 MakePerspectiveFovMatrix(
		float fovY,
		float aspectRatio,
		float nearClip,
		float farClip);

	// 正射影
	Matrix4x4 MakeOrthographicMatrix(
		float left,
		float top,
		float right,
		float bottom,
		float nearClip,
		float farClip);

	// ビューポート
	Matrix4x4 MakeViewportMatrix(
		float left,
		float top,
		float width,
		float height,
		float minDepth,
		float maxDepth);
}