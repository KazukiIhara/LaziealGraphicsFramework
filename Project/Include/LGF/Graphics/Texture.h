#pragma once

#include <cstdint>
#include <string>

namespace LGF {

	enum class TextureColorSpace : uint8_t {
		SRGB,
		Linear,
	};

	struct TextureLoadDesc {
		TextureColorSpace colorSpace = TextureColorSpace::SRGB;
		bool generateMipMaps = true;
	};

	class Texture {
	public:
		Texture() = default;
		explicit Texture(const std::string& filePath, const TextureLoadDesc& desc = {});

		bool IsValid() const;
		uint32_t Width() const;
		uint32_t Height() const;
		bool IsCubeMap() const;

	private:
		struct Handle {
			uint32_t index = 0;
			uint32_t generation = 0;
		};

		explicit Texture(Handle handle);

		Handle handle_{};

		friend class TextureManager;
	};

}
