#pragma once

#include <array>

#include "LGF/Math/MathTypes.h"

#include "Transform2D.h"
#include "Color.h"

namespace LGF {

	/// <summary>
	/// 2D三角形
	/// </summary>
	class Triangle2D {
	public:
		// デフォルト三角形を生成
		Triangle2D();
		// 形状を指定して三角形を生成
		Triangle2D(const Vector2& v0, const Vector2& v1, const Vector2& v2);

		~Triangle2D();

		void Draw(
			const Matrix3x3& worldMatrix,
			const Vector4& color = Color::White
		)const;

	private:
		// 頂点 左下→上→右下
		std::array<Vector2, 3> vertices{
			Vector2{-50.0f,-50.0f},
			Vector2{0.0f,50.0f},
			Vector2{50.0f,-50.0f}
		};
	};

}