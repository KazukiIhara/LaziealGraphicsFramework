#pragma once

#include <cstdint>

#include "LGF/Math/Matrix3x3.h" 
#include "LGF/Graphics/Color.h"

namespace LGF {

	class Camera2D;

	class Triangle2D;
	class Rect2D;
	class Circle2D;
	class Line2D;

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
			const Vector4& color= Color::White
		);

		/// <summary>
		/// 四角形描画
		/// </summary>
		void Draw(
			const Rect2D& rect,
			const Vector4& color= Color::White
		);

		/// <summary>
		/// 円描画
		/// </summary>
		void Draw(
			const Circle2D& circle,
			const Vector4& color= Color::White
		);

		/// <summary>
		/// 線分描画
		/// </summary>
		void Draw(
			const Line2D& line,
			const Vector4& color= Color::White
		);

	}
}
