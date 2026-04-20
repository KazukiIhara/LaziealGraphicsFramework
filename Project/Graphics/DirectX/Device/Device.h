#pragma once

#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "ComPtr/ComPtr.h"

namespace LGF {
	/// <summary>
	/// DirectXデバイス管理
	class Device {
	public:
		// 初期化結果
		enum class InitializeResult {
			kSuccess,
			kFactoryCreateFailed,
			kAdapterNotFound,
			kDeviceCreateFailed,
			kMeshShaderNotSupported,
			kDevice10NotAvailable,
		};

	public:
		// コンストラクタ
		Device();
		// デストラクタ
		~Device() = default;

		// 初期化
		InitializeResult Initialize();

		// ディスクリプタヒープの作成
		ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
			D3D12_DESCRIPTOR_HEAP_TYPE heapType,
			UINT numDescriptors,
			bool shaderVisible);


		// DXGIデバイスの取得
		ID3D12Device* GetDevice()const;
		// DXGIデバイス10の取得
		ID3D12Device10* GetDevice10()const;
		// DXGIファクトリーの取得
		IDXGIFactory7* GetFactory()const;

	private:
		// デバッグレイヤーを有効化
		void EnableDebugLayer();
		// DXGIファクトリーを作成
		bool CreateFactory();
		// アダプタを選択
		bool SelectAdapter();
		// デバイスを作成
		bool CreateDevice();
		// MeshShader対応を確認
		InitializeResult CheckMeshShaderSupport();
		// InfoQueueを設定
		void SetupInfoQueue();

	private:
		// DXGIファクトリー
		ComPtr <IDXGIFactory7> dxgiFactory_ = nullptr;
		// 使用するアダプタ用の変数
		ComPtr<IDXGIAdapter4> adapter_ = nullptr;
		// Device
		ComPtr<ID3D12Device> device_ = nullptr;
		// MeshShader対応Device
		ComPtr<ID3D12Device10> device10_ = nullptr;

	};
}