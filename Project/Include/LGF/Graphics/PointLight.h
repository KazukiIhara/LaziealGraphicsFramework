#pragma once

#include "LGF/Graphics/Color.h"
#include "LGF/Math/Vector3.h"

namespace LGF {

	/// <summary>
	/// 指定した位置から周囲を照らす点光源
	/// </summary>
	class PointLight {
	public:
		PointLight() = default;
		PointLight(
			const Vector3& position,
			const Vector4& color = Color::White,
			float intensity = 1.0f,
			float range = 10.0f);

		/// <summary>
		/// このフレームで使用する点光源として設定
		/// </summary>
		void SetScene() const;

		Vector3 position = {};
		Vector4 color = Color::White;
		float intensity = 1.0f;
		float range = 10.0f;
	};

}
