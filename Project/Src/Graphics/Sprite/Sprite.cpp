#include "LGF/Graphics/Sprite.h"

#include "LGF/Graphics/Material.h"
#include "LGF/Graphics/Rect2D.h"

using namespace LGF;

namespace {
	Vector2 GetTextureSize(const Texture& texture) {
		return {
			static_cast<float>(texture.GetWidth()),
			static_cast<float>(texture.GetHeight())
		};
	}
}

Sprite::Sprite() = default;

Sprite::Sprite(const Texture& texture) :
	texture(texture),
	size(GetTextureSize(texture)) {
}

Sprite::Sprite(const Texture& texture, const Vector2& leftTop) :
	texture(texture),
	leftTop(leftTop),
	size(GetTextureSize(texture)) {
}

Sprite::Sprite(const Texture& texture, const Vector2& leftTop, const Vector2& size) :
	texture(texture),
	leftTop(leftTop),
	size(size) {
}

Sprite::~Sprite() = default;

void Sprite::Draw(const Vector4& color) const {
	Rect2D{ leftTop, size }.Draw(Material2D{
		.color = color,
		.baseColorTexture = texture,
		.uvScale = uvScale,
		.uvOffset = uvOffset,
	});
}
