#pragma once

#include "LGF/Math/MathTypes.h"

#include "Color.h"
#include "Texture.h"

namespace LGF {

	/// <summary>
	/// 2Dスプライト
	/// </summary>
	class Sprite {
	public:
		Sprite();
		// テクスチャ本来のサイズでスプライトを生成
		explicit Sprite(const Texture& texture);
		// 左上座標とテクスチャ本来のサイズでスプライトを生成
		Sprite(const Texture& texture, const Vector2& leftTop);
		// 左上座標と表示サイズを指定してスプライトを生成
		Sprite(const Texture& texture, const Vector2& leftTop, const Vector2& size);

		~Sprite();

		void Draw(const Vector4& color = Color::White)const;

	public:
		Texture texture;
		Vector2 leftTop{};
		Vector2 size{};
		Vector2 uvScale{ 1.0f, 1.0f };
		Vector2 uvOffset{};
	};

}
