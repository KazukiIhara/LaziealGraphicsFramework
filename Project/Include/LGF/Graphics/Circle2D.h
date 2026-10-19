#pragma once

#include "LGF/Math/MathTypes.h"

#include "Color.h"

namespace LGF {

	struct Material2D;

	/// <summary>
	/// 2D円
	/// </summary>
	class Circle2D {
	public:
		/// <summary>デフォルトの円を生成</summary>
		Circle2D();
		/// <summary>中心座標と半径を指定して円を生成</summary>
		Circle2D(const Vector2& center, float radius);
		/// <summary>中心座標と半径を指定して円を生成</summary>
		Circle2D(float x, float y, float radius);

		~Circle2D();

		void Draw(const Vector4& color = Color::White) const;
		void Draw(const Material2D& material) const;

	public:
		Vector2 center{};
		float radius{};
	};

}
