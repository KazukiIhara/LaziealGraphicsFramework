#pragma once

#include <cstdint>
#include <string>

namespace LGF {

	enum class TextureDimension : uint8_t {
		Auto,
		Texture2D,
		TextureCube,
	};

	enum class TextureColorSpace : uint8_t {
		SRGB,
		Linear,
	};

	struct TextureLoadDesc {
		TextureDimension dimension = TextureDimension::Auto;
		TextureColorSpace colorSpace = TextureColorSpace::SRGB;
		bool generateMipMaps = true;

		static TextureLoadDesc Color();
		static TextureLoadDesc NormalMap();
		static TextureLoadDesc Cubemap();
	};

	/// <summary>
	/// テクスチャクラス
	/// </summary>
	class Texture {
	public:
		Texture(const std::string& filePath, const TextureLoadDesc& desc = {});
		~Texture();

		const std::string& GetFilePath()const;

		// テクスチャのデータは名前をキーにマネージャから持ってくる

	private:
		std::string filePath_;
	};
}
