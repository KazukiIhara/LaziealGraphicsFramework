#pragma once

#include "LGF/Math/MathTypes.h"
#include "Color.h"

namespace LGF::Graphics3D {

	/// <summary>
	/// マテリアルデータ
	/// </summary>
	struct Material {
		Vector4 color = Color::White;
	};

}