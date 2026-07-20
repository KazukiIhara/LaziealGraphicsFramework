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

	struct TextureData {
		ComPtr<ID3D12Resource> resource;
		DirectX::TexMetadata metaData;
		uint32_t srvIndex;
	};

	/// <summary>
	/// テクスチャマネージャ
	/// </summary>
	class TextureManager {
	public:
		TextureManager();
		~TextureManager();

		bool Initialize();

		void Finalize();

		void Load(const std::string& filePath, const TextureLoadDesc& desc);

		void LoadAll();

		const TextureData& Find(const std::string& filePath);

	private:
		std::unordered_map<std::string, TextureData> textures_;
	};

}