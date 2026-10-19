#pragma once

#include <cstdint>

#include "LGF/Graphics/Color.h"

namespace LGF {

	class Camera2D;
	struct Material2D;

	class Triangle2D;
	class Rect2D;
	class Circle2D;
	class Line2D;
	class RoundRect2D;

	namespace Graphics2D {

		/// <summary>
		/// 現在のウィンドウの横幅を取得
		/// </summary>
		uint32_t GetWindowWidth();

		/// <summary>
		/// 現在のウィンドウの縦幅を取得
		/// </summary>
		uint32_t GetWindowHeight();

		/// <summary>
		/// 使用するカメラをセットする
		/// </summary>
		void SetCamera(const Camera2D& camera);

		/// <summary>
		/// 三角形描画
		/// </summary>
		void Draw(
			const Triangle2D& triangle,
			const Vector4& color = Color::White);
		void Draw(const Triangle2D& triangle, const Material2D& material);

		/// <summary>
		/// 四角形描画
		/// </summary>
		void Draw(
			const Rect2D& rect,
			const Vector4& color = Color::White);
		void Draw(const Rect2D& rect, const Material2D& material);

		/// <summary>
		/// 円描画
		/// </summary>
		void Draw(
			const Circle2D& circle,
			const Vector4& color = Color::White);
		void Draw(const Circle2D& circle, const Material2D& material);

		/// <summary>
		/// 線分描画
		/// </summary>
		void Draw(
			const Line2D& line,
			const Vector4& color = Color::White);
		void Draw(const Line2D& line, const Material2D& material);

		/// <summary>
		/// 角丸四角形描画
		/// </summary>
		void Draw(
			const RoundRect2D& roundRect,
			const Vector4& color = Color::White);
		void Draw(const RoundRect2D& roundRect, const Material2D& material);

	}
}
