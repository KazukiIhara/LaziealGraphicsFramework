#pragma once

#include "LGF/Math/MathTypes.h"

#include "Transform3D.h"
#include "Color.h"
#include "Material.h"

namespace LGF {

	/// <summary>
	/// 3D球体
	/// </summary>
	class Sphere3D {
	public:
		Sphere3D();
		explicit Sphere3D(float radius);

		~Sphere3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		)const;

		void Draw(const Matrix4x4& worldMatrix, const Material3D& material) const;

	public:
		float radius = 0.5f;
	};
}
