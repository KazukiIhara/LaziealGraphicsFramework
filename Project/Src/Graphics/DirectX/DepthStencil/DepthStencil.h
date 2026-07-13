#pragma once

#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

namespace LGF {

	class ResourceFactory;
	class DSVManager;

	/// <summary>
	/// DepthStencilクラス
	/// </summary>
	class DepthStencil {
	public:
		// コンストラクタ
		DepthStencil(ResourceFactory& resourceFactory, DSVManager& dsvManager);
		// デストラクタ
		~DepthStencil() = default;

		// 初期化
		void Initialize(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT resourceFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
			DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);
		void Resize(uint32_t width, uint32_t height);

		// Resourceの取得
		ID3D12Resource* GetResource()const;
		// DSVIndexの取得
		uint32_t GetDSVIndex()const;
		// DSVのCPUハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandleCPU()const;

	private:
		// Resource
		ComPtr<ID3D12Resource> resource_ = nullptr;
		// DSVManager
		DSVManager& dsvManager_;
		// ResourceFactory
		ResourceFactory& resourceFactory_;
		// DSVIndex
		uint32_t dsvIndex_ = 0u;
		DXGI_FORMAT resourceFormat_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
		DXGI_FORMAT dsvFormat_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
		bool isDsvAllocated_ = false;
	};
}
