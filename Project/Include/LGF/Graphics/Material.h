#pragma once

#include <cstdint>

#include "LGF/Math/MathTypes.h"
#include "Color.h"
#include "Texture.h"

namespace LGF {

	enum class ShadingModel3D : uint8_t {
		SceneDefault,
		PBR,
		Toon,
	};

	struct ToonMaterial3D {
		Vector4 shadowColor = { 0.20f, 0.16f, 0.32f, 1.0f };
		Vector4 middleColor = { 0.65f, 0.55f, 0.75f, 1.0f };
		Vector4 lightColor = Color::White;
		float shadowThreshold = 0.35f;
		float lightThreshold = 0.70f;
		float smoothness = 0.02f;
	};

	/// <summary>
	/// 2Dマテリアルデータ
	/// </summary>
	struct Material2D {
		Vector4 color = Color::White;
		// ベースカラーはtexture * color。未指定ならcolorのみを使用する
		Texture baseColorTexture;
		// UVはscale / offset適用後、リピートと線形ミップフィルタでサンプリングする
		Vector2 uvScale = { 1.0f, 1.0f };
		Vector2 uvOffset{};
	};

	/// <summary>
	/// マテリアルデータ
	/// </summary>
	struct Material3D {
		Vector4 color = Color::White;
		// ベースカラーはtexture * color。未指定ならcolorのみを使用する
		Texture baseColorTexture;
		// タンジェント空間法線。TextureLoadDesc::NormalMap()でLinear読み込みする
		Texture normalTexture;
		// glTF準拠：G=Roughness、B=Metallic。データテクスチャとしてLinear読み込みする
		Texture metallicRoughnessTexture;
		// glTF準拠：R=Ambient Occlusion。metallicRoughnessTextureと同じ画像でもよい
		Texture occlusionTexture;
		// 0で無効、1でテクスチャ本来の強さ
		float normalScale = 1.0f;
		float metallic = 0.0f;
		float roughness = 1.0f;
		float occlusionStrength = 1.0f;
		// SceneDefaultはシーン全体のLighting設定に従う
		ShadingModel3D shadingModel = ShadingModel3D::SceneDefault;
		ToonMaterial3D toonLighting{};
		// trueならライト計算を行わず、ベースカラーをそのまま出力する
		bool unlit = false;
		// UVはscale / offset適用後、リピートと線形ミップフィルタでサンプリングする
		Vector2 uvScale = { 1.0f, 1.0f };
		Vector2 uvOffset{};
	};

}
