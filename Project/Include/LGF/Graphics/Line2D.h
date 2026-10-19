#pragma once

#include "LGF/Math/MathTypes.h"

#include "Color.h"

namespace LGF {
	struct Material2D;

	/// <summary>
	/// 2D線分
	/// </summary>
	class Line2D {
	public:
		// デフォルト線分を生成
		Line2D();
		// 始点、終点、太さを指定して線分を生成
		Line2D(const Vector2& start, const Vector2& end, float thickness = 1.0f);
		// 始点、終点、太さを指定して線分を生成
		Line2D(float x0, float y0, float x1, float y1, float thickness = 1.0f);

		~Line2D();

		void Draw(const Vector4& color = Color::White)const;
		void Draw(const Material2D& material)const;

	public:
		Vector2 start{};
		Vector2 end{};
		float thickness{};
	};

}
