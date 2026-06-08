#pragma once

#include "Math/Types/Vector2.h"
#include "Math/Types/Vector3.h"
#include "Math/Types/Vector4.h"
#include "Math/Types/Matrix2x2.h"
#include "Math/Types/Matrix3x3.h"
#include "Math/Types/Matrix4x4.h"
#include "Math/Types/Quaternion.h"

namespace LGF {
	/// <summary>
	/// GPUに送る3D三角形データ
	/// </summary>
	struct Triangle3DForGPU {
		Matrix4x4 worldMatrix;
		Matrix4x4 worldInverseTranspose;
		Vector4 color;
		Vector4 Offsets[2];
	};
}

