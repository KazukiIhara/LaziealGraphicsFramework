#pragma once

#include <cstddef>
#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "ComPtr/ComPtr.h"

namespace LGF {

	class Device;

	/// <summary>
	/// DirectXリソース生成クラス
	/// </summary>
	class ResourceFactory {
	public:
		// コンストラクタ
		ResourceFactory(Device& device);
		// デストラクタ
		~ResourceFactory() = default;

		ResourceFactory(const ResourceFactory&) = delete;
		ResourceFactory& operator=(const ResourceFactory&) = delete;
		ResourceFactory(ResourceFactory&&) = delete;
		ResourceFactory& operator=(ResourceFactory&&) = delete;

		// Uploadバッファリソースの作成
		ComPtr<ID3D12Resource> CreateUploadBufferResource(size_t sizeInBytes);
		// UAVバッファリソースの作成
		ComPtr<ID3D12Resource> CreateUAVBufferResource(size_t sizeInBytes);

		// デプスステンシルリソースの作成
		ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(
			int32_t width,
			int32_t height,
			DXGI_FORMAT format);

		// デプスステンシルリソースの作成
		ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(
			int32_t width,
			int32_t height,
			DXGI_FORMAT format,
			DXGI_FORMAT clearFormat);

	private:
		// Device
		Device& device_;
	};
}
