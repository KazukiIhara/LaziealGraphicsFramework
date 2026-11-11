#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>


#include <d3d12.h>
#include <DirectXTex.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

#include "LGF/Graphics/Texture.h"

namespace LGF {
	class DirectXCore;

	struct TextureData {
		ComPtr<ID3D12Resource> resource;
		DirectX::TexMetadata metaData{};
		uint32_t srvIndex = UINT32_MAX;
	};

	/// <summary>
	/// テクスチャマネージャ
	/// </summary>
	class TextureManager {
	public:
		explicit TextureManager(DirectXCore& directX);
		~TextureManager();

		bool Initialize();

		void Finalize();

		const TextureData& Load(const std::filesystem::path& filePath, const TextureLoadDesc& desc);
		const TextureData& LoadFromMemory(
			const std::wstring& cacheKey,
			const void* data,
			size_t size,
			const TextureLoadDesc& desc
		);
		const TextureData& LoadFromPixels(
			const std::wstring& cacheKey,
			const void* pixels,
			uint32_t width,
			uint32_t height,
			size_t rowPitch,
			const TextureLoadDesc& desc
			);

	private:
		const TextureData& Store(
			std::wstring key,
			const std::string& debugName,
			DirectX::ScratchImage image,
			const TextureLoadDesc& desc
		);

		std::unordered_map<std::wstring, TextureData> textures_;
		DirectXCore& directX_;
	};

}
