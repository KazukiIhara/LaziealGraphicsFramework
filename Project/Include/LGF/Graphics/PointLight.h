#pragma once

#include <cstdint>

#include "LGF/Graphics/Color.h"
#include "LGF/Math/Vector3.h"

namespace LGF {

	/// <summary>
	/// 指定した位置から周囲を照らす点光源
	/// </summary>
	class PointLight {
	public:
		/// <summary>
		/// 1フレームに設定できるポイントライトの最大数
		/// </summary>
		static constexpr uint32_t MaxCount = 64u;

		PointLight() = default;
		PointLight(
			const Vector3& position,
			const Vector4& color = Color::White,
			float intensity = 1.0f,
			float range = 10.0f);

		/// <summary>
		/// このフレームで使用する点光源として追加
		/// </summary>
		void SetScene() const;

		Vector3 position = {};
		Vector4 color = Color::White;
		float intensity = 1.0f;
		float range = 10.0f;
	};

}
