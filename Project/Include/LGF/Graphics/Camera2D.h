#pragma once

#include "LGF/Math/MathTypes.h"

namespace LGF {

	/// <summary>
	/// 2Dカメラ
	/// </summary>
	class Camera2D {
	public:
		Camera2D();
		~Camera2D();

		void Set()const;

	public:
		Vector2 translation{};
		float rotation{};
		Vector2 size{};
	};

}