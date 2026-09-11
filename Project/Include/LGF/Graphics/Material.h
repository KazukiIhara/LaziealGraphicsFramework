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
		// UV は scale / offset 適用後、リピート＋線形ミップフィルタでサンプリング。
		Vector2 uvScale = { 1.0f, 1.0f };
		Vector2 uvOffset = {};
	};

}
