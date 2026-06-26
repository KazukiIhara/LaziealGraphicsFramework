#pragma once

#include <cstdint>

#include "LGF/Math/Matrix3x3.h" 
#include "LGF/Graphics/Color.h"

namespace LGF {

	class Camera2D;

	class Triangle2D;

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
		/// 三角形描画
		/// </summary>
		void Draw(
			const Triangle2D& triangle,
			const Matrix3x3& worldMatrix,
			const Vector4& color= Color::White
		);

	}
}