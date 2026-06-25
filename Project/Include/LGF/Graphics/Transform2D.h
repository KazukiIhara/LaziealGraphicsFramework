#pragma once

#include "LGF/Math/MathTypes.h"

namespace LGF {
	/// <summary>
	/// 2Dトランスフォーム
	/// </summary>
	struct Transform2D {
		Vector2 scale{ 1.0f,1.0f };
		float rotation{};
		Vector2 translation{};
	};

}