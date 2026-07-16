#pragma once

#include "LGF/Math/MathTypes.h"

#include "Transform3D.h"
#include "Color.h"

namespace LGF {

	/// <summary>
	/// 3D箱
	/// </summary>
	class Box3D {
	public:
		Box3D();
		explicit Box3D(float size);
		Box3D(float width, float height, float depth);

		~Box3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		)const;

	public:
		float width = 1.0f;
		float height = 1.0f;
		float depth = 1.0f;
	};
}
