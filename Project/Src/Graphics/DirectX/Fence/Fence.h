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
		uint64_t Signal();
		// GPUを待機
		void Wait(uint64_t fenceValue);
		void WaitIdle();
		bool IsCompleted(uint64_t fenceValue) const;
		bool IsInitialized() const;

	private:
		Device& device_;
		Command& cmd_;

		// フェンス
		ComPtr<ID3D12Fence> fence_ = nullptr;
		// フェンスバリュー
		UINT64 fenceValue_ = 0;
		void* fenceEvent_ = nullptr;

	};
}
