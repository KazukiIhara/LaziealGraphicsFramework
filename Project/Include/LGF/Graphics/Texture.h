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

	/// <summary>
	/// テクスチャクラス
	/// </summary>
	class Texture {
	public:
		Texture(const std::string& filePath, const TextureLoadDesc& desc = {});
		~Texture();

		const std::string& GetFilePath()const;
		uint32_t GetWidth()const;
		uint32_t GetHeight()const;
		uint32_t GetSrvIndex()const;

	private:
		std::string filePath_;
		uint32_t width_ = 0;
		uint32_t height_ = 0;
		bool isCubemap_ = false;
		uint32_t srvIndex_ = 0;
	};
}
