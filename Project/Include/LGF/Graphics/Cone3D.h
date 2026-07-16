#pragma once

#include "LGF/Math/MathTypes.h"

#include "Transform3D.h"
#include "Color.h"

namespace LGF {

	class Cone3D {
	public:
		Cone3D();
		Cone3D(float radius, float height);

		~Cone3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		)const;

	public:
		float radius = 0.5f;
		float height = 1.0f;
	};

}
