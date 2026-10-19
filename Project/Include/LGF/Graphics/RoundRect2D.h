#pragma once

#include "LGF/Math/MathTypes.h"

#include "Color.h"

namespace LGF {

	struct Material2D;

	/// <summary>
	/// 2D角丸四角形
	/// </summary>
	class RoundRect2D {
	public:
		/// <summary>デフォルトの角丸四角形を生成</summary>
		RoundRect2D();
		/// <summary>左上座標、サイズ、角丸半径を指定して角丸四角形を生成</summary>
		RoundRect2D(const Vector2& leftTop, const Vector2& size, float radius);
		/// <summary>左上座標、幅、高さ、角丸半径を指定して角丸四角形を生成</summary>
		RoundRect2D(float x, float y, float width, float height, float radius);

		~RoundRect2D();

		void Draw(const Vector4& color = Color::White) const;
		void Draw(const Material2D& material) const;

	public:
		Vector2 leftTop{};
		Vector2 size{};
		float radius{};
	};

}
