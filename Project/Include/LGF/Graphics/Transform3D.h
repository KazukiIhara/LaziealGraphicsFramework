#pragma once

#include "LGF/Math/MathTypes.h"

namespace LGF {
	/// <summary>
	/// 3Dトランスフォーム
	/// </summary>
	struct Transform3D {
		Vector3 scale{ 1.0f,1.0f,1.0f };
		Quaternion rotation{};
		Vector3 translation{};
	};

}