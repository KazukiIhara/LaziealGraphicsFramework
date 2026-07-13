#pragma once

#include "LGF/Math/MathTypes.h"

#include "Transform3D.h"
#include "Color.h"

namespace LGF {

	class Plane3D {
	public:
		Plane3D();
		explicit Plane3D(float size);
		Plane3D(float width, float height);

		~Plane3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		)const;

	public:
		float width = 1.0f;
		float height = 1.0f;
	};
}
