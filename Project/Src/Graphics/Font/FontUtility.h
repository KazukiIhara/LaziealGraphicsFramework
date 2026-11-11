#pragma once

#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <string>
#include <string_view>

#include "LGF/Math/Vector2.h"

namespace LGF::FontUtility {

	inline bool IsValidAnchor(const Vector2& anchor) {
		return std::isfinite(anchor.x) && std::isfinite(anchor.y) &&
			anchor.x >= 0.0f && anchor.x <= 1.0f &&
			anchor.y >= 0.0f && anchor.y <= 1.0f;
	}

	inline std::u32string DecodeUtf8(std::string_view text) {
		std::u32string result;
		result.reserve(text.size());

		for (size_t index = 0; index < text.size();) {
			const uint8_t first = static_cast<uint8_t>(text[index]);
			char32_t codePoint{};
			size_t length{};
			if (first < 0x80u) {
				codePoint = first;
				length = 1;
			} else if ((first & 0xE0u) == 0xC0u) {
				codePoint = first & 0x1Fu;
				length = 2;
			} else if ((first & 0xF0u) == 0xE0u) {
				codePoint = first & 0x0Fu;
				length = 3;
			} else if ((first & 0xF8u) == 0xF0u) {
				codePoint = first & 0x07u;
				length = 4;
			} else {
				throw std::invalid_argument("Text contains invalid UTF-8.");
			}

			if (index + length > text.size()) {
				throw std::invalid_argument("Text contains truncated UTF-8.");
			}
			for (size_t continuation = 1; continuation < length; ++continuation) {
				const uint8_t value = static_cast<uint8_t>(text[index + continuation]);
				if ((value & 0xC0u) != 0x80u) {
					throw std::invalid_argument("Text contains invalid UTF-8 continuation bytes.");
				}
				codePoint = (codePoint << 6u) | (value & 0x3Fu);
			}

			const bool overlong =
				(length == 2 && codePoint < 0x80u) ||
				(length == 3 && codePoint < 0x800u) ||
				(length == 4 && codePoint < 0x10000u);
			if (overlong || codePoint > 0x10FFFFu ||
				(codePoint >= 0xD800u && codePoint <= 0xDFFFu)) {
				throw std::invalid_argument("Text contains an invalid Unicode code point.");
			}

			result.push_back(codePoint);
			index += length;
		}
		return result;
	}

	inline uint64_t MakeKerningKey(char32_t first, char32_t second) {
		return (static_cast<uint64_t>(first) << 32u) | static_cast<uint32_t>(second);
	}

}
