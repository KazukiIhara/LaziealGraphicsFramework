#pragma once

#include <d3d12.h>

#include "ComPtr/ComPtr.h"

namespace LGF {

	class Device;

	/// <summary>
	/// DescriptorHeap管理クラス
	/// </summary>
	class DescriptorHeapManager {
	public:
		// コンストラクタ
		DescriptorHeapManager(Device& device);
		// デストラクタ
		~DescriptorHeapManager() = default;

		DescriptorHeapManager(const DescriptorHeapManager&) = delete;
		DescriptorHeapManager& operator=(const DescriptorHeapManager&) = delete;
		DescriptorHeapManager(DescriptorHeapManager&&) = delete;
		DescriptorHeapManager& operator=(DescriptorHeapManager&&) = delete;

		// DescriptorHeapの作成
		ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
			D3D12_DESCRIPTOR_HEAP_TYPE heapType,
			UINT numDescriptors,
			bool shaderVisible);

	private:
		// Device
		Device& device_;
	};
}
