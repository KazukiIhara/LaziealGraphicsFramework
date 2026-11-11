#include "FontLayout.h"

#include <algorithm>

#include "FontResource.h"
#include "FontUtility.h"

using namespace LGF;

FontTextLayout LGF::BuildFontTextLayout(
	const FontResource& resource,
	std::string_view text) {

	FontTextLayout layout{};
	const std::u32string codePoints = FontUtility::DecodeUtf8(text);
	if (codePoints.empty()) {
		return layout;
	}

	Vector2 pen{ 0.0f, resource.ascent };
	float maximumWidth{};
	float height = resource.lineHeight;
	char32_t previous{};
	for (const char32_t original : codePoints) {
		if (original == U'\r') {
			continue;
		}
		if (original == U'\n') {
			maximumWidth = std::max(maximumWidth, pen.x);
			pen.x = 0.0f;
			pen.y += resource.lineHeight;
			height += resource.lineHeight;
			previous = 0;
			continue;
		}
		if (original == U'\t') {
			pen.x += resource.glyphs.at(U' ').advance * 4.0f;
			previous = U' ';
			continue;
		}

		const char32_t codePoint = resource.glyphs.contains(original) ? original : U'?';
		if (previous != 0) {
			const auto iterator = resource.kerning.find(
				FontUtility::MakeKerningKey(previous, codePoint));
			if (iterator != resource.kerning.end()) {
				pen.x += iterator->second;
			}
		}

		const FontGlyph& glyph = resource.glyphs.at(codePoint);
		if (glyph.size.x > 0.0f && glyph.size.y > 0.0f) {
			layout.glyphs.push_back({
				.leftTop = pen + glyph.offset,
				.size = glyph.size,
				.uvScale = glyph.uvScale,
				.uvOffset = glyph.uvOffset,
			});
		}
		pen.x += glyph.advance;
		previous = codePoint;
	}

	layout.size = { std::max(maximumWidth, pen.x), height };
	return layout;
}
