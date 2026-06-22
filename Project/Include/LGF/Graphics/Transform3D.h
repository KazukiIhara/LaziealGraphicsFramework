#pragma once

#include "LGF/Math/MathTypes.h"

namespace LGF {
	struct Transform3D {
		Vector3 scale{ 1.0f,1.0f,1.0f };
		Vector3 rotation;
		Vector3 translation;
	};
}