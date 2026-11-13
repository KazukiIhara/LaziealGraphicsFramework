#pragma once

#include "LGF/Graphics/Color.h"
#include "LGF/Math/MathTypes.h"

namespace LGF {

	/// <summary>3D UIの背景へ使用する両面描画の板</summary>
	class UIPanel3D {
	public:
		UIPanel3D();
		explicit UIPanel3D(float size);
		UIPanel3D(float width, float height);

		~UIPanel3D();

		/// <summary>ライティングの影響を受けない色で両面描画</summary>
		void Draw(
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White) const;

	public:
		float width = 1.0f;
		float height = 1.0f;
	};

}
