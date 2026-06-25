#pragma once

#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

namespace LGF {

	class Device;

	/// <summary>
	/// DescriptorHeap管理クラス
	/// </summary>
	class DescriptorHeapFactory {
	public:
		// コンストラクタ
		DescriptorHeapFactory(Device& device);
		// デストラクタ
		~DescriptorHeapFactory() = default;

		DescriptorHeapFactory(const DescriptorHeapFactory&) = delete;
		DescriptorHeapFactory& operator=(const DescriptorHeapFactory&) = delete;
		DescriptorHeapFactory(DescriptorHeapFactory&&) = delete;
		DescriptorHeapFactory& operator=(DescriptorHeapFactory&&) = delete;

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
