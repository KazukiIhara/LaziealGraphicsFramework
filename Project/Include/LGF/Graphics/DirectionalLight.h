#pragma once

#include "LGF/Graphics/Color.h"
#include "LGF/Math/Vector3.h"

namespace LGF {

	/// <summary>
	/// シーン全体を照らす平行光源
	/// </summary>
	class DirectionalLight {
	public:
		DirectionalLight() = default;
		DirectionalLight(const Vector3& direction, const Vector4& color = Color::White, float intensity = 1.0f);

		/// <summary>
		/// このフレームで使用する平行光源として設定
		/// </summary>
		void SetScene() const;

		// 光が進む向き。GPUへ送る際に正規化される。
		Vector3 direction = { 0.0f, -1.0f, 0.0f };
		Vector4 color = Color::White;
		float intensity = 1.0f;
	};

}
