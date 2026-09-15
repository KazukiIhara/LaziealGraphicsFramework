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
		// 未指定のテクスチャ。Material3D では単色描画になる。
		Texture() = default;
		// UTF-8 パスを同期ロード。失敗時は std::exception 派生の例外を送出する。
		Texture(const std::string& filePath, const TextureLoadDesc& desc = {});
		~Texture();

		const std::string& GetFilePath()const;
		bool IsValid() const { return srvIndex_ != UINT32_MAX; }

		// GPU リソースはフレームワーク終了まで内部で共有する。

	private:
		friend class Renderer3D;
		friend class SkyboxSystem;
		std::string filePath_;
		uint32_t srvIndex_ = UINT32_MAX;
		bool isCubeMap_ = false;
		ID3D12Resource* resource_ = nullptr;
	};
}
