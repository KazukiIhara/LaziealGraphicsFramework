#pragma once

#include <array>

#include "LGF/Math/MathTypes.h"

#include "Transform3D.h"
#include "Color.h"

namespace LGF {

	/// <summary>
	/// 3D三角形
	/// </summary>
	class Triangle3D {
	public:
		// デフォルト三角形を生成
		Triangle3D();
		// 形状を指定して三角形を生成
		Triangle3D(const Vector3& v0, const Vector3& v1, const Vector3& v2);

		~Triangle3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		)const;

	public:
		std::array<Vector3, 3> vertices{
			Vector3{ -1.0f, -1.0f, 0.0f },
			Vector3{  0.0f,  1.0f, 0.0f },
			Vector3{  1.0f, -1.0f, 0.0f }
		};
	};
}