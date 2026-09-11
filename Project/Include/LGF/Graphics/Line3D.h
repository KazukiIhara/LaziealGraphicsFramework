#pragma once

#include "LGF/Math/MathTypes.h"

#include "Transform3D.h"
#include "Color.h"
#include "Material.h"

namespace LGF {

	class Line3D {
	public:
		Line3D();
		Line3D(const Vector3& start, const Vector3& end);
		Line3D(float x0, float y0, float z0, float x1, float y1, float z1);

		~Line3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		)const;

		void Draw(const Matrix4x4& worldMatrix, const Material3D& material) const;

	public:
		Vector3 start{};
		Vector3 end{};
	};

}
