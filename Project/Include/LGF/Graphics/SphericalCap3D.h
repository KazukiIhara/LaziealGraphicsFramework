#pragma once

#include "LGF/Math/MathTypes.h"

#include "Color.h"
#include "Material.h"

namespace LGF {

	/// <summary>
	/// 円形の境界を持つ、閉じていない球面。
	/// ローカル原点を頂点とし、+Y方向へ height 進んだ位置に
	/// 半径 baseRadius の円形境界を持つ。法線は球面の内側を向く。
	/// </summary>
	class SphericalCap3D {
	public:
		SphericalCap3D();
		SphericalCap3D(float baseRadius, float height);

		~SphericalCap3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		) const;

		void Draw(const Matrix4x4& worldMatrix, const Material3D& material) const;

	public:
		float baseRadius = 1.0f;
		float height = 0.5f;
	};

}
