#include "FontManager.h"

#include <algorithm>
#include <format>
#include <fstream>
#include <stdexcept>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include <imstb_truetype.h>
#undef STB_TRUETYPE_IMPLEMENTATION
#undef STBTT_STATIC
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "FontResource.h"
#include "FontLayout.h"
#include "FontUtility.h"
#include "Graphics/Texture/TextureManager.h"
#include "LGF/Graphics/Texture.h"

using namespace LGF;

namespace {

	constexpr uint32_t kInitialAtlasSize = 256u;
	constexpr uint32_t kMaximumAtlasSize = 8192u;

	std::string ToUtf8String(const std::filesystem::path& path) {
		const auto value = path.u8string();
		return std::string(value.begin(), value.end());
	}

	stbtt_fontinfo CreateFontInfo(const FontResource& resource) {
		stbtt_fontinfo info{};
		const int offset = stbtt_GetFontOffsetForIndex(resource.fileData.data(), 0);
		if (offset < 0 || !stbtt_InitFont(&info, resource.fileData.data(), offset)) {
			throw std::runtime_error("Font data is invalid or unsupported.");
		}
		return info;
	}

	char32_t ResolveCodePoint(const FontResource& resource, char32_t codePoint) {
		if (resource.glyphs.contains(codePoint)) {
			return codePoint;
		}
		return U'?';
	}

}

FontManager::FontManager(TextureManager& textureManager) :
	textureManager_(textureManager) {
}

FontManager::~FontManager() = default;

bool FontManager::Initialize() {
	return true;
}

void FontManager::Finalize() {
	fonts_.clear();
}

std::shared_ptr<FontResource> FontManager::Load(
	const std::filesystem::path& filePath,
	uint32_t pixelSize) {

	if (filePath.empty()) {
		throw std::invalid_argument("Font path must not be empty.");
	}
	if (pixelSize == 0) {
		throw std::invalid_argument("Font pixel size must be greater than zero.");
	}

	const auto path = std::filesystem::absolute(filePath).lexically_normal();
	const auto key = std::format(L"{}|{}", path.native(), pixelSize);
	if (const auto iterator = fonts_.find(key); iterator != fonts_.end()) {
		return iterator->second;
	}

	std::ifstream stream(path, std::ios::binary | std::ios::ate);
	if (!stream) {
		throw std::runtime_error(std::format(
			"Failed to open font '{}'.",
			ToUtf8String(filePath)));
	}
	const std::streamsize fileSize = stream.tellg();
	if (fileSize <= 0) {
		throw std::runtime_error("Font file is empty.");
	}
	stream.seekg(0, std::ios::beg);

	auto resource = std::make_shared<FontResource>();
	resource->filePath = filePath;
	resource->pixelSize = pixelSize;
	resource->fileData.resize(static_cast<size_t>(fileSize));
	if (!stream.read(
		reinterpret_cast<char*>(resource->fileData.data()),
		fileSize)) {
		throw std::runtime_error("Failed to read font file.");
	}

	const stbtt_fontinfo info = CreateFontInfo(*resource);
	int ascent{};
	int descent{};
	int lineGap{};
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
	resource->scale = stbtt_ScaleForPixelHeight(&info, static_cast<float>(pixelSize));
	resource->ascent = static_cast<float>(ascent) * resource->scale;
	resource->lineHeight = static_cast<float>(ascent - descent + lineGap) * resource->scale;

	resource->codePoints.reserve(95);
	for (char32_t codePoint = U' '; codePoint <= U'~'; ++codePoint) {
		resource->codePoints.push_back(codePoint);
	}
	RebuildAtlas(*resource);

	fonts_.emplace(key, resource);
	return resource;
}

void FontManager::PrepareText(
	const std::shared_ptr<FontResource>& resource,
	std::string_view text) {

	if (!resource) {
		throw std::invalid_argument("Font resource must not be empty.");
	}

	const std::u32string codePoints = FontUtility::DecodeUtf8(text);
	const stbtt_fontinfo info = CreateFontInfo(*resource);
	bool requiresRebuild = false;
	for (const char32_t codePoint : codePoints) {
		if (codePoint == U'\n' || codePoint == U'\r' || codePoint == U'\t' ||
			resource->glyphs.contains(codePoint)) {
			continue;
		}
		if (stbtt_FindGlyphIndex(&info, static_cast<int>(codePoint)) == 0) {
			continue;
		}
		resource->codePoints.push_back(codePoint);
		requiresRebuild = true;
	}

	if (requiresRebuild) {
		std::sort(resource->codePoints.begin(), resource->codePoints.end());
		resource->codePoints.erase(
			std::unique(resource->codePoints.begin(), resource->codePoints.end()),
			resource->codePoints.end());
		RebuildAtlas(*resource);
	}

	char32_t previous{};
	for (const char32_t original : codePoints) {
		if (original == U'\n' || original == U'\r') {
			previous = 0;
			continue;
		}
		if (original == U'\t') {
			previous = U' ';
			continue;
		}
		const char32_t current = ResolveCodePoint(*resource, original);
		if (previous != 0) {
			const uint64_t key = FontUtility::MakeKerningKey(previous, current);
			if (!resource->kerning.contains(key)) {
				const float amount = static_cast<float>(stbtt_GetCodepointKernAdvance(
					&info,
					static_cast<int>(previous),
					static_cast<int>(current))) * resource->scale;
				resource->kerning.emplace(key, amount);
			}
		}
		previous = current;
	}
}

Vector2 FontManager::Measure(
	const std::shared_ptr<FontResource>& resource,
	std::string_view text) {

	PrepareText(resource, text);
	return BuildFontTextLayout(*resource, text).size;
}

void FontManager::RebuildAtlas(FontResource& resource) {
	std::vector<int> codePoints;
	codePoints.reserve(resource.codePoints.size());
	for (const char32_t codePoint : resource.codePoints) {
		codePoints.push_back(static_cast<int>(codePoint));
	}

	std::vector<stbtt_packedchar> packedCharacters(codePoints.size());
	std::vector<uint8_t> coverage;
	uint32_t atlasSize = kInitialAtlasSize;
	bool packed = false;
	while (atlasSize <= kMaximumAtlasSize) {
		std::fill(packedCharacters.begin(), packedCharacters.end(), stbtt_packedchar{});
		coverage.assign(static_cast<size_t>(atlasSize) * atlasSize, 0u);
		stbtt_pack_context context{};
		if (!stbtt_PackBegin(
			&context,
			coverage.data(),
			static_cast<int>(atlasSize),
			static_cast<int>(atlasSize),
			0,
			1,
			nullptr)) {
			throw std::runtime_error("Failed to initialize the font atlas.");
		}
		stbtt_PackSetSkipMissingCodepoints(&context, 1);

		stbtt_pack_range range{};
		range.font_size = static_cast<float>(resource.pixelSize);
		range.array_of_unicode_codepoints = codePoints.data();
		range.num_chars = static_cast<int>(codePoints.size());
		range.chardata_for_range = packedCharacters.data();
		packed = stbtt_PackFontRanges(
			&context,
			resource.fileData.data(),
			0,
			&range,
			1) != 0;
		stbtt_PackEnd(&context);
		if (packed) {
			break;
		}
		atlasSize *= 2u;
	}
	if (!packed) {
		throw std::runtime_error("Font glyphs do not fit in the maximum atlas size.");
	}

	resource.glyphs.clear();
	for (size_t index = 0; index < resource.codePoints.size(); ++index) {
		const stbtt_packedchar& packedCharacter = packedCharacters[index];
		const float width = static_cast<float>(packedCharacter.x1 - packedCharacter.x0);
		const float height = static_cast<float>(packedCharacter.y1 - packedCharacter.y0);
		resource.glyphs.emplace(resource.codePoints[index], FontGlyph{
			.size = { width, height },
			.offset = { packedCharacter.xoff, packedCharacter.yoff },
			.uvScale = { width / atlasSize, height / atlasSize },
			.uvOffset = {
				static_cast<float>(packedCharacter.x0) / atlasSize,
				static_cast<float>(packedCharacter.y0) / atlasSize,
			},
			.advance = packedCharacter.xadvance,
		});
	}

	std::vector<uint8_t> rgba(static_cast<size_t>(atlasSize) * atlasSize * 4u);
	for (size_t index = 0; index < coverage.size(); ++index) {
		rgba[index * 4u + 0u] = 255u;
		rgba[index * 4u + 1u] = 255u;
		rgba[index * 4u + 2u] = 255u;
		rgba[index * 4u + 3u] = coverage[index];
	}

	++resource.atlasRevision;
	const auto path = std::filesystem::absolute(resource.filePath).lexically_normal();
	const std::wstring cacheKey = std::format(
		L"font:{}|{}|{}",
		path.native(),
		resource.pixelSize,
		resource.atlasRevision);
	TextureLoadDesc textureDesc = TextureLoadDesc::Data();
	textureDesc.generateMipMaps = false;
	const TextureData& texture = textureManager_.LoadFromPixels(
		cacheKey,
		rgba.data(),
		atlasSize,
		atlasSize,
		static_cast<size_t>(atlasSize) * 4u,
		textureDesc);
	resource.atlasSrvIndex = texture.srvIndex;
	resource.atlasWidth = atlasSize;
	resource.atlasHeight = atlasSize;
}
