#pragma once

#include "LGF/Math/MathTypes.h"

namespace LGF {

	/// <summary>
	/// 2Dカメラ
	/// </summary>
	class Camera2D {
	public:
		/// <summary>
		/// デフォルトのカメラを作成
		/// </summary>
		Camera2D();

		/// <summary>
		/// 姿勢を指定してカメラを作成
		/// </summary>
		Camera2D(
			Vector2 translation,
			float rotation,
			Vector2 size);

		~Camera2D();

		void SetScene() const;

	public:
		Vector2 translation{};
		float rotation{};
		Vector2 size{};
	};

}
