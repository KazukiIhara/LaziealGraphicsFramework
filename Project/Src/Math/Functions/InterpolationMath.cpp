#include "LGF/Math/Functions/InterpolationMath.h"

namespace LGF::Math {

	float Lerp(float start, float end, float t) {
		return start + (end - start) * t;
	}

	Vector2 Lerp(const Vector2& start, const Vector2& end, float t) {
		return start + (end - start) * t;
	}

	Vector3 Lerp(const Vector3& start, const Vector3& end, float t) {
		return start + (end - start) * t;
	}

	Vector4 Lerp(const Vector4& start, const Vector4& end, float t) {
		return start + (end - start) * t;
	}

}
