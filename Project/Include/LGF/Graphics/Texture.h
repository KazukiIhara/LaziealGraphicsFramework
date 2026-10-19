#pragma once

#include <cstdint>
#include <string>

struct ID3D12Resource;

namespace LGF {
	class SkyboxSystem;

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
		static TextureLoadDesc Data();
		static TextureLoadDesc NormalMap();
		static TextureLoadDesc Cubemap();
	};

	/// <summary>
	/// テクスチャクラス
	/// </summary>
	class Texture {
	public:
		/// <summary>マテリアルで単色描画に使用する空のテクスチャを生成</summary>
		Texture() = default;
		/// <summary>UTF-8パスからテクスチャを同期読み込み。失敗時は例外を送出</summary>
		Texture(const std::string& filePath, const TextureLoadDesc& desc = {});
		~Texture();

		const std::string& GetFilePath() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		bool IsValid() const { return srvIndex_ != UINT32_MAX; }

	private:
		friend class Renderer3D;
		friend class Renderer2D;
		friend class SkyboxSystem;
		std::string filePath_;
		uint32_t srvIndex_ = UINT32_MAX;
		bool isCubeMap_ = false;
		ID3D12Resource* resource_ = nullptr;
	};

}
