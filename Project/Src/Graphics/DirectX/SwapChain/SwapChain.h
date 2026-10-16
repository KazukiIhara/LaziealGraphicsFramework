#pragma once

#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "Graphics/DirectX/FrameContext/FrameContext.h"

#include "Graphics/DirectX/ComPtr/ComPtr.h"

namespace LGF {
	class Device;
	class Command;
	class RTVManager;
	class Viewport;
	class ScissorRect;

	/// <summary>
	/// スワップチェーン
	/// </summary>
	class SwapChain {
	public:
		SwapChain(
			void* windowHandle,
			uint32_t width,
			uint32_t height,
			Device& device,
			Command& cmd,
			RTVManager& rtvMgr,
			Viewport& viewport,
			ScissorRect& scissorRect,
			bool vsyncEnabled);
		~SwapChain();

		bool Initialize();
		bool Resize(uint32_t width, uint32_t height);
		void BeginFrame();
		void EndFrame();
		void Present();

		void TransitionToRenderTarget();
		void TransitionToPresent();

		ID3D12Resource* GetCurrentBackBufferResource();
		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTVHandle();
		uint32_t GetCurrentBackBufferIndex() const;

	private:
		bool CreateSwapChain();
		bool CreateResources();
		void CreateRTV();

	private:
		void* windowHandle_ = nullptr;
		Device& device_;
		Command& cmd_;
		RTVManager& rtvMgr_;
		Viewport& viewport_;
		ScissorRect& scissorRect_;

		// スワップチェーン
		ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
		// スワップチェーンリソース
		ComPtr<ID3D12Resource> swapChainResources_[kFrameContextCount]{};
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
		// RTVリソースのインデックス
		uint32_t rtvIndex_[kFrameContextCount]{};
		bool isRtvAllocated_ = false;
		// バックバッファインデックス
		UINT backBufferIndex_ = 0;
		bool vsyncEnabled_ = true;
		bool tearingSupported_ = false;
	};

}
