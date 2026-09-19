#pragma once

#include <array>
#include <cstdint>

#include <d3d12.h>

#include "Graphics/DirectX/FrameContext/FrameContext.h"

#include "Graphics/DirectX/ComPtr/ComPtr.h"

namespace LGF {

	class Device;

	/// <summary>
	/// DirectXCommandクラス
	/// </summary>
	class Command {
	public:
		Command(Device& device);
		~Command();

		bool Initialize();
		void BeginFrame(uint32_t frameContextIndex);
		void EndFrame();

		// コマンドの実行
		void KickCommand();
		// コマンドのリセット
		void ResetCommand(uint32_t frameContextIndex);
		FrameContext& GetFrameContext(uint32_t frameContextIndex);
		// キューの取得
		ID3D12CommandQueue* GetQueue();
		// リストの取得
		ID3D12GraphicsCommandList* GetList();
		// メッシュシェーダー用リストの取得
		ID3D12GraphicsCommandList6* GetList6();

	private:
		Device& device_;

		// コマンドキュー
		ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
		// コマンドアロケータ
		std::array<FrameContext, kFrameContextCount> frameContexts_{};
		// コマンドリスト
		ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
		// MeshShader用コマンドリスト
		ComPtr<ID3D12GraphicsCommandList6> commandList6_ = nullptr;
	};
}
