#pragma once

#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "BaseViewManager/BaseViewManager.h"

namespace LGF {

	class Device;
	class DescriptorHeapFactory;

	/// <summary>
	/// SRV/UAV管理クラス
	/// </summary>
	class SRVUAVManager : public BaseViewManager {
	public:
		// コンストラクタ
		SRVUAVManager(Device& device, DescriptorHeapFactory& descriptorHeapManager);
		// デストラクタ
		~SRVUAVManager() override = default;

		// 初期化
		void Initialize(uint32_t maxViewCount = 1024u);
		// Texture2D用のSRVを作成
		void CreateSrvTexture2d(uint32_t viewIndex, ID3D12Resource* resource, DXGI_FORMAT format, UINT mipLevels);
		// CubeMap用のSRVを作成
		void CreateSrvTextureCubeMap(uint32_t viewIndex, ID3D12Resource* resource, DXGI_FORMAT format);
		// StructuredBuffer用のSRVを作成
		void CreateSrvStructuredBuffer(uint32_t viewIndex, ID3D12Resource* resource, uint32_t numElements, UINT structureByteStride);
		// ByteAddressBuffer用のSRVを作成
		void CreateSrvByteAddressBuffer(uint32_t viewIndex, ID3D12Resource* resource, uint32_t byteSize);
		// StructuredBuffer用のUAVを作成
		void CreateUavStructuredBuffer(uint32_t viewIndex, ID3D12Resource* resource, uint32_t numElements, UINT structureByteStride);
		// CBVを作成
		void CreateConstantBufferView(uint32_t viewIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, uint32_t sizeInBytes);

	private:
		// Device
		Device& device_;
	};
}
