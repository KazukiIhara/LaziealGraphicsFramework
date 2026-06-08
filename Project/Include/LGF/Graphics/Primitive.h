#pragma once

#include "LGF/Math/MathTypes.h"

namespace LGF::Graphics3D {
	/// <summary>
	/// 三角形データ構造体
	/// </summary>
	struct Triangle {
		Vector3 vertices[3]{
			{ -1.0f,-1.0f,0.0f },
			{ 0.0f,1.0f,0.0f },
			{ 1.0f,-1.0f,0.0f },
		};
	};

}