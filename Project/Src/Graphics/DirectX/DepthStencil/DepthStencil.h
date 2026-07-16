#pragma once

#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

namespace LGF {

	class ResourceFactory;
	class DSVManager;
	class SRVUAVManager;

	/// <summary>
	/// DepthStencilクラス
	/// </summary>
	class DepthStencil {
	public:
		// コンストラクタ
		DepthStencil(ResourceFactory& resourceFactory, DSVManager& dsvManager, SRVUAVManager& srvUavManager);
		// デストラクタ
		~DepthStencil() = default;

		// 初期化
		void Initialize(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT resourceFormat = DXGI_FORMAT_R24G8_TYPELESS,
			DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
			DXGI_FORMAT srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
		void Resize(uint32_t width, uint32_t height);
		void Transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES nextState);

		// Resourceの取得
		ID3D12Resource* GetResource()const;
		// DSVIndexの取得
		uint32_t GetDSVIndex()const;
		uint32_t GetSRVIndex()const;
		// DSVのCPUハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandleCPU()const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandleGPU()const;

	private:
		// Resource
		ComPtr<ID3D12Resource> resource_ = nullptr;
		// DSVManager
		DSVManager& dsvManager_;
		SRVUAVManager& srvUavManager_;
		// ResourceFactory
		ResourceFactory& resourceFactory_;
		// DSVIndex
		uint32_t dsvIndex_ = 0u;
		uint32_t srvIndex_ = 0u;
		DXGI_FORMAT resourceFormat_ = DXGI_FORMAT_R24G8_TYPELESS;
		DXGI_FORMAT dsvFormat_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
		DXGI_FORMAT srvFormat_ = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		bool isDsvAllocated_ = false;
		bool isSrvAllocated_ = false;
		D3D12_RESOURCE_STATES currentState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	};
}
