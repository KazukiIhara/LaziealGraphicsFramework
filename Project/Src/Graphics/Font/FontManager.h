#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "LGF/Math/Vector2.h"

namespace LGF {

	class TextureManager;
	struct FontResource;

	class FontManager {
	public:
		explicit FontManager(TextureManager& textureManager);
		~FontManager();

		bool Initialize();
		void Finalize();

		std::shared_ptr<FontResource> Load(
			const std::filesystem::path& filePath,
			uint32_t pixelSize);
		void PrepareText(const std::shared_ptr<FontResource>& resource, std::string_view text);
		Vector2 Measure(const std::shared_ptr<FontResource>& resource, std::string_view text);

	private:
		void RebuildAtlas(FontResource& resource);

		TextureManager& textureManager_;
		std::unordered_map<std::wstring, std::shared_ptr<FontResource>> fonts_;
	};

}
