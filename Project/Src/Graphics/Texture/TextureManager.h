#pragma once

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
	class SRVUAVManager;

	class TextureManager {
	public:
		explicit TextureManager(DirectXCore& directX);
		~TextureManager();

		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;

		bool Initialize();
		void Finalize();

		Texture Load(const std::string& filePath, const TextureLoadDesc& desc = {});
		const Texture& GetDefaultTexture() const;

		bool IsValid(const Texture& texture) const;
		uint32_t GetWidth(const Texture& texture) const;
		uint32_t GetHeight(const Texture& texture) const;
		bool IsCubeMap(const Texture& texture) const;
		uint32_t GetSrvIndex(const Texture& texture) const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const Texture& texture) const;

	private:
		struct Entry {
			ComPtr<ID3D12Resource> resource;
			DirectX::TexMetadata metadata{};
			uint32_t srvIndex = 0;
			uint32_t generation = 1;
		};

		bool InitializeUploadContext();
		Texture CreateDefaultTexture();
		Texture LoadImpl(const std::filesystem::path& filePath, const TextureLoadDesc& desc);
		bool LoadTextureImage(const std::filesystem::path& filePath, const TextureLoadDesc& desc, DirectX::ScratchImage& image) const;
		bool CreateTexture(const DirectX::ScratchImage& image, Entry& entry);
		ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata) const;
		bool UploadTexture(ID3D12Resource* texture, const DirectX::ScratchImage& image);
		bool ExecuteUpload();
		std::string MakeCacheKey(const std::filesystem::path& filePath, const TextureLoadDesc& desc) const;
		const Entry* FindEntry(const Texture& texture) const;
		Texture MakeTexture(uint32_t index) const;

		DirectXCore& directX_;
		SRVUAVManager* srvUavManager_ = nullptr;
		std::vector<Entry> entries_;
		std::unordered_map<std::string, uint32_t> cache_;
		Texture defaultTexture_{};

		ComPtr<ID3D12CommandAllocator> uploadAllocator_;
		ComPtr<ID3D12GraphicsCommandList> uploadCommandList_;
		ComPtr<ID3D12Fence> uploadFence_;
		HANDLE uploadFenceEvent_ = nullptr;
		uint64_t uploadFenceValue_ = 0;
	};

}
