#pragma once

#include <string_view>
#include <vector>

#include "LGF/Math/Vector2.h"

namespace LGF {

	struct FontResource;

	struct FontGlyphLayout {
		Vector2 leftTop{};
		Vector2 size{};
		Vector2 uvScale{};
		Vector2 uvOffset{};
	};

	struct FontTextLayout {
		std::vector<FontGlyphLayout> glyphs;
		Vector2 size{};
	};

	FontTextLayout BuildFontTextLayout(
		const FontResource& resource,
		std::string_view text);

}
