#pragma once

#include "LGF/Math/MathTypes.h"

#include "Color.h"

namespace LGF {
	struct Material2D;

	/// <summary>
	/// 2D四角形
	/// </summary>
	class Rect2D {
	public:
		// デフォルト四角形を生成
		Rect2D();
		// 左上座標とサイズを指定して四角形を生成
		Rect2D(const Vector2& leftTop, const Vector2& size);
		// 左上座標と幅、高さを指定して四角形を生成
		Rect2D(float x, float y, float width, float height);

		~Rect2D();

		void Draw(const Vector4& color = Color::White)const;
		void Draw(const Material2D& material)const;

	public:
		Vector2 leftTop{};
		Vector2 size{};
	};

}
