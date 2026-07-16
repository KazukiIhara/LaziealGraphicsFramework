#pragma once

#include "LGF/Math/MathTypes.h"

#include "Transform3D.h"
#include "Color.h"

namespace LGF {

	class Capsule3D {
	public:
		Capsule3D();
		Capsule3D(float radius, float height);

		~Capsule3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		)const;

	public:
		float radius = 0.25f;
		float height = 1.0f;
	};

}
