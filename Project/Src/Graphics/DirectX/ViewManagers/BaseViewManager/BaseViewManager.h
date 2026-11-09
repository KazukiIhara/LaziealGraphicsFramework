#pragma once

#include <cstdint>
#include <vector>

#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

namespace LGF {

	class Device;
	class DescriptorHeapFactory;

	/// <summary>
	/// View管理の基底クラス
	/// </summary>
	class BaseViewManager {
	public:
		// コンストラクタ
		BaseViewManager(Device& device, DescriptorHeapFactory& descriptorHeapManager);
		// デストラクタ
		virtual ~BaseViewManager() = default;

		BaseViewManager(const BaseViewManager&) = delete;
		BaseViewManager& operator=(const BaseViewManager&) = delete;
		BaseViewManager(BaseViewManager&&) = delete;
		BaseViewManager& operator=(BaseViewManager&&) = delete;

		// 初期化
		void Initialize(
			D3D12_DESCRIPTOR_HEAP_TYPE heapType,
			uint32_t maxViewCount,
			bool shaderVisible);

		// ViewIndexの割り当て
		uint32_t Allocate();
		// GPUがViewを参照し終えた後にIndexを再利用可能へ戻す。
		void Release(uint32_t index);
		// CPUハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index)const;
		// GPUハンドルを取得
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index)const;
		// DescriptorHeapの取得
		ID3D12DescriptorHeap* GetDescriptorHeap()const;
		// DescriptorSizeの取得
		uint32_t GetDescriptorSize()const;
		// 使用数の取得
		uint32_t GetUseIndex()const;
		// Viewの最大数を下回っているか
		bool IsLowerViewMax()const;
		bool CanAllocate(uint32_t count) const;

	private:
		// Device
		Device& device_;
		// DescriptorHeapManager
		DescriptorHeapFactory& descriptorHeapManager_;
		// DescriptorHeap
		ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;
		// Descriptor1個分のサイズ
		uint32_t descriptorSize_ = 0u;
		// 使用しているViewIndex
		uint32_t useIndex_ = 0u;
		std::vector<uint32_t> freeIndices_;
		// Viewの最大数
		uint32_t maxViewCount_ = 0u;
		// ShaderVisibleかどうか
		bool isShaderVisible_ = false;
	};
}
