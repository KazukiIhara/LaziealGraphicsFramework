#pragma once

#include "LGF/Math/MathTypes.h"

#include "Color.h"
#include "Material.h"

namespace LGF {

	/// <summary>
	/// 同心の内外球面と、開口部の環状面で構成する厚みのある球殻。
	/// ローカル原点を外側球面の頂点とし、+Y方向へ height 進んだ位置に
	/// 共通の開口平面を持つ。
	/// </summary>
	class SphericalShellCap3D {
	public:
		SphericalShellCap3D();
		SphericalShellCap3D(float outerRadius, float innerRadius, float height);

		~SphericalShellCap3D();

		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		) const;

		void Draw(const Matrix4x4& worldMatrix, const Material3D& material) const;

	public:
		float outerRadius = 2.0f;
		float innerRadius = 1.8f;
		float height = 1.0f;
	};

}
