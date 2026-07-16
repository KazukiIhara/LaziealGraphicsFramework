#pragma once

#include "LGF/Math/MathTypes.h"

#include "Transform3D.h"
#include "Color.h"

namespace LGF {

	class Ground3D {
	public:
		Ground3D();
		explicit Ground3D(float size);
		Ground3D(float width, float depth);

		~Ground3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		)const;

	public:
		float width = 1.0f;
		float depth = 1.0f;
	};
}
