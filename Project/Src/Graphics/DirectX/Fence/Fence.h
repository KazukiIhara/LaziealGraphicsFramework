#pragma once

#include <cstdint>

#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

namespace LGF {

	class Device;
	class Command;

	/// <summary>
	/// DirectXFenceクラス
	/// </summary>
	class Fence {
	public:
		Fence(Device& device, Command& cmd);
		~Fence();

		bool Initialize();
		void EndFrame();
		// GPUを待機
		void WaitGPU();

	private:
		Device& device_;
		Command& cmd_;

		// フェンス
		ComPtr<ID3D12Fence> fence_ = nullptr;
		// フェンスバリュー
		UINT64 fenceValue_ = 0;

	};
}
