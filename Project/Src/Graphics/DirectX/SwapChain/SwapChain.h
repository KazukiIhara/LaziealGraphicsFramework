#pragma once

#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "ComPtr/ComPtr.h"

namespace LGF {
	class Window;
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
		SwapChain(Window& win, Device& device, Command& cmd, RTVManager& rtvMgr, Viewport& viewport, ScissorRect& scissorRect);
		~SwapChain();

		bool Initialize();
		void Present();

		void TransitionToRenderTarget();
		void TransitionToPresent();

		ID3D12Resource* GetCurrentBackBufferResource();
		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTVHandle();

	private:
		bool CreateSwapChain();
		bool CreateResources();
		void CreateRTV();

	private:
		Window& win_;
		Device& device_;
		Command& cmd_;
		RTVManager& rtvMgr_;
		Viewport& viewport_;
		ScissorRect& scissorRect_;

		// スワップチェーン
		ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
		// スワップチェーンリソース
		ComPtr<ID3D12Resource> swapChainResources_[2] = { nullptr };
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
		// RTVリソースのインデックス
		uint32_t rtvIndex_[2]{};
		// バックバッファインデックス
		UINT backBufferIndex_ = 0;
	};

}