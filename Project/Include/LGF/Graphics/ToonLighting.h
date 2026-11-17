#pragma once

#include "LGF/Graphics/Color.h"

namespace LGF {

	/// <summary>
	/// シーン全体へ適用するToon Lighting設定
	/// </summary>
	class ToonLighting {
	public:
		/// <summary>
		/// このフレームで使用するToon Lightingとして設定
		/// </summary>
		void SetScene() const;

		bool enabled = true;
		Vector4 shadowColor = { 0.20f, 0.16f, 0.32f, 1.0f };
		Vector4 middleColor = { 0.65f, 0.55f, 0.75f, 1.0f };
		Vector4 lightColor = Color::White;
		float shadowThreshold = 0.35f;
		float lightThreshold = 0.70f;
		float smoothness = 0.02f;
	};

}
