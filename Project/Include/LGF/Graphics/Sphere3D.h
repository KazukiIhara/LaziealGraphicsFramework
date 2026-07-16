#pragma once

#include "LGF/Math/MathTypes.h"

#include "Transform3D.h"
#include "Color.h"

namespace LGF {

	class Sphere3D {
	public:
		Sphere3D();
		explicit Sphere3D(float radius);

		~Sphere3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		)const;

	public:
		float radius = 0.5f;
	};
}
