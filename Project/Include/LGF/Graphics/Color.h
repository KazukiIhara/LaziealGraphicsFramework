#pragma once

#include "Math/Vector4.h"

namespace LGF::Color {

	// 赤系
	inline constexpr Vector4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	inline constexpr Vector4 Crimson = { 0.86f, 0.08f, 0.24f, 1.0f };
	inline constexpr Vector4 Tomato = { 1.0f, 0.39f, 0.28f, 1.0f };
	inline constexpr Vector4 Coral = { 1.0f, 0.5f, 0.31f, 1.0f };
	inline constexpr Vector4 OrangeRed = { 1.0f, 0.27f, 0.0f, 1.0f };

	// ピンク系
	inline constexpr Vector4 Pink = { 1.0f, 0.75f, 0.8f, 1.0f };
	inline constexpr Vector4 HotPink = { 1.0f, 0.41f, 0.71f, 1.0f };
	inline constexpr Vector4 DeepPink = { 1.0f, 0.08f, 0.58f, 1.0f };

	// 青系
	inline constexpr Vector4 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	inline constexpr Vector4 SkyBlue = { 0.53f, 0.81f, 0.92f, 1.0f };
	inline constexpr Vector4 RoyalBlue = { 0.25f, 0.41f, 0.88f, 1.0f };
	inline constexpr Vector4 CornflowerBlue = { 0.39f, 0.58f, 0.93f, 1.0f };
	inline constexpr Vector4 DeepSkyBlue = { 0.0f, 0.75f, 1.0f, 1.0f };
	inline constexpr Vector4 DodgerBlue = { 0.12f, 0.56f, 1.0f, 1.0f };
	inline constexpr Vector4 AliceBlue = { 0.94f, 0.97f, 1.0f, 1.0f };
	inline constexpr Vector4 Navy = { 0.0f, 0.0f, 0.5f, 1.0f };

	// シアン系
	inline constexpr Vector4 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	inline constexpr Vector4 Aqua = { 0.0f, 1.0f, 1.0f, 1.0f };
	inline constexpr Vector4 Turquoise = { 0.25f, 0.88f, 0.82f, 1.0f };

	// 緑系
	inline constexpr Vector4 Green = { 0.0f, 0.5f, 0.0f, 1.0f };
	inline constexpr Vector4 Lime = { 0.0f, 1.0f, 0.0f, 1.0f };
	inline constexpr Vector4 ForestGreen = { 0.13f, 0.55f, 0.13f, 1.0f };
	inline constexpr Vector4 Olive = { 0.5f, 0.5f, 0.0f, 1.0f };
	inline constexpr Vector4 SpringGreen = { 0.0f, 1.0f, 0.5f, 1.0f };
	inline constexpr Vector4 LawnGreen = { 0.49f, 0.99f, 0.0f, 1.0f };
	inline constexpr Vector4 Chartreuse = { 0.5f, 1.0f, 0.0f, 1.0f };

	// 黄系
	inline constexpr Vector4 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	inline constexpr Vector4 Gold = { 1.0f, 0.84f, 0.0f, 1.0f };
	inline constexpr Vector4 Khaki = { 0.94f, 0.9f, 0.55f, 1.0f };
	inline constexpr Vector4 LemonChiffon = { 1.0f, 0.98f, 0.8f, 1.0f };

	// オレンジ系
	inline constexpr Vector4 Orange = { 1.0f, 0.65f, 0.0f, 1.0f };
	inline constexpr Vector4 DarkOrange = { 1.0f, 0.55f, 0.0f, 1.0f };

	// 紫系
	inline constexpr Vector4 Purple = { 0.5f, 0.0f, 0.5f, 1.0f };
	inline constexpr Vector4 Violet = { 0.93f, 0.51f, 0.93f, 1.0f };
	inline constexpr Vector4 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
	inline constexpr Vector4 Indigo = { 0.29f, 0.0f, 0.51f, 1.0f };

	// 茶系
	inline constexpr Vector4 Brown = { 0.65f, 0.16f, 0.16f, 1.0f };
	inline constexpr Vector4 Chocolate = { 0.82f, 0.41f, 0.12f, 1.0f };
	inline constexpr Vector4 Beige = { 0.96f, 0.96f, 0.86f, 1.0f };

	// 白・黒系
	inline constexpr Vector4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	inline constexpr Vector4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	inline constexpr Vector4 Gray = { 0.5f, 0.5f, 0.5f, 1.0f };
	inline constexpr Vector4 LightGray = { 0.83f, 0.83f, 0.83f, 1.0f };
	inline constexpr Vector4 DarkGray = { 0.33f, 0.33f, 0.33f, 1.0f };
	inline constexpr Vector4 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };

	// 特殊
	inline constexpr Vector4 Transparent = { 0.0f, 0.0f, 0.0f, 0.0f };

}