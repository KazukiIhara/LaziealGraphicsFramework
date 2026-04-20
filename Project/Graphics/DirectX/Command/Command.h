#pragma once

#include <d3d12.h>

#include "ComPtr/ComPtr.h"

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

		// コマンドの実行
		void KickCommand();
		// コマンドのリセット
		void ResetCommand();
		// キューの取得
		ID3D12CommandQueue* GetQueue();
		// アロケータの取得
		ID3D12CommandAllocator* GetAllocator();
		// リストの取得
		ID3D12GraphicsCommandList* GetList();
		// メッシュシェーダー用リストの取得
		ID3D12GraphicsCommandList6* GetList6();

	private:
		Device& device_;

		// コマンドキュー
		ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
		// コマンドアロケータ
		ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
		// コマンドリスト
		ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
		// MeshShader用コマンドリスト
		ComPtr<ID3D12GraphicsCommandList6> commandList6_ = nullptr;
	};
}