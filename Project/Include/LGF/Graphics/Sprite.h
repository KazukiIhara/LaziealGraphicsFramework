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
		/// <summary>空のスプライトを生成</summary>
		Sprite();
		/// <summary>テクスチャ本来のサイズでスプライトを生成</summary>
		explicit Sprite(const Texture& texture);
		/// <summary>左上座標とテクスチャ本来のサイズでスプライトを生成</summary>
		Sprite(const Texture& texture, const Vector2& leftTop);
		/// <summary>左上座標と表示サイズを指定してスプライトを生成</summary>
		Sprite(const Texture& texture, const Vector2& leftTop, const Vector2& size);

		~Sprite();

		void Draw(const Vector4& color = Color::White) const;

	public:
		Texture texture;
		Vector2 leftTop{};
		Vector2 size{};
		Vector2 uvScale{ 1.0f, 1.0f };
		Vector2 uvOffset{};
	};

}
