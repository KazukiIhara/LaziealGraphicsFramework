#pragma once

#include "LGF/Math/Vector2.h"
#include "LGF/Math/Vector3.h"
#include "LGF/Math/Vector4.h"

namespace LGF::Math {

	// 線形補間
	float Lerp(float start, float end, float t);
	Vector2 Lerp(const Vector2& start, const Vector2& end, float t);
	Vector3 Lerp(const Vector3& start, const Vector3& end, float t);
	Vector4 Lerp(const Vector4& start, const Vector4& end, float t);

}
