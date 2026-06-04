#pragma once

#include <cstdint>

namespace LGF::Graphics {
	/// <summary>
	/// ブレンドモード
	/// </summary>
	enum class BlendMode {
		None,
		Normal,
		Add,
		Subtract,
		Multiply,
		Screen,

		Num, // 使用禁止
	};

	// ブレンドモードの種類の数
	inline constexpr uint32_t kBlendModeNum = static_cast<uint32_t>(BlendMode::Num);
}