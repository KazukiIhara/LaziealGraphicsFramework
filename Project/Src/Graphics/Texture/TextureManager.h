#pragma once

#include <string>
#include <cstdint>
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

		const TextureData& Load(const std::string& filePath, const TextureLoadDesc& desc);

	private:
		std::unordered_map<std::wstring, TextureData> textures_;
		DirectXCore& directX_;
	};

}
