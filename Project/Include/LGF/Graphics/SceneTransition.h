#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "LGF/Math/MathTypes.h"

#include "Color.h"

namespace LGF {

	inline constexpr uint32_t kSceneTransitionParameterCount = 4u;

	/// <summary>
	/// シーン描画後に重ねる遷移用フルスクリーンエフェクトの描画設定。
	/// </summary>
	struct SceneTransitionDrawDesc {
		std::string pipeline = "SceneTransitionFade";
		Vector4 color = Color::Black;
		/// <summary>カスタムシェーダーのgParametersに渡す汎用パラメータ。</summary>
		std::array<Vector4, kSceneTransitionParameterCount> parameters{};
		float progress = 0.0f;
		float phaseProgress = 0.0f;
		float opacity = 0.0f;
		float phase = 0.0f;
	};

}
