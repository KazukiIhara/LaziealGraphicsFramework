#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "LGF/Math/Vector2.h"

namespace LGF {

	struct FontGlyph {
		Vector2 size{};
		Vector2 offset{};
		Vector2 uvScale{};
		Vector2 uvOffset{};
		float advance{};
	};

	struct FontResource {
		std::filesystem::path filePath;
		std::vector<uint8_t> fileData;
		std::vector<char32_t> codePoints;
		std::unordered_map<char32_t, FontGlyph> glyphs;
		std::unordered_map<uint64_t, float> kerning;
		uint32_t pixelSize{};
		uint32_t atlasSrvIndex = UINT32_MAX;
		uint32_t atlasWidth{};
		uint32_t atlasHeight{};
		uint32_t atlasRevision{};
		float scale{};
		float ascent{};
		float lineHeight{};
	};

}
