#pragma once

#include "LGF/Math/MathTypes.h"
#include "Color.h"
#include "Texture.h"

namespace LGF{

	/// <summary>
	/// マテリアルデータ
	/// </summary>
	struct Material3D {
		Vector4 color = Color::White;
		// ベースカラーは texture * color。未指定なら color のみを使用する。
		Texture baseColorTexture;
		// タンジェント空間法線。TextureLoadDesc::NormalMap() でLinear読み込みする。
		Texture normalTexture;
		// glTF準拠: G=Roughness、B=Metallic。データテクスチャとしてLinear読み込みする。
		Texture metallicRoughnessTexture;
		// glTF準拠: R=Ambient Occlusion。metallicRoughnessTextureと同じ画像でもよい。
		Texture occlusionTexture;
		// 0で無効、1でテクスチャ本来の強さ。
		float normalScale = 1.0f;
		float metallic = 0.0f;
		float roughness = 1.0f;
		float occlusionStrength = 1.0f;
		// trueならライト計算を行わず、ベースカラーをそのまま出力する。
		bool unlit = false;
		// UV は scale / offset 適用後、リピート＋線形ミップフィルタでサンプリング。
		Vector2 uvScale = { 1.0f, 1.0f };
		Vector2 uvOffset = {};
	};

}
