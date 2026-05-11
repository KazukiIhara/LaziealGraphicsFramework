#pragma once

#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "ComPtr/ComPtr.h"

namespace LGF {
	/// <summary>
	/// DirectXデバイス管理
	/// </summary>
	class Device {
	public:
		// デバイスの対応機能
		struct DeviceCaps {
			bool isMeshShaderSupported = false;
			bool isDevice10Available = false;
		};

		// 初期化結果
		enum class InitializeResult {
			kSuccess,
			kFactoryCreateFailed,
			kAdapterNotFound,
			kDeviceCreateFailed,
		};

	public:
		// コンストラクタ
		Device();
		// デストラクタ
		~Device() = default;

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&&) = delete;
		Device& operator=(Device&&) = delete;

		// 初期化
		InitializeResult Initialize();

		// DXGIデバイスの取得
		ID3D12Device* GetDevice()const;
		// DXGIデバイス10の取得
		ID3D12Device10* GetDevice10()const;
		// DXGIファクトリーの取得
		IDXGIFactory7* GetFactory()const;
		// デバイス対応機能の取得
		const DeviceCaps& GetDeviceCaps()const;

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
		void CheckMeshShaderSupport();
		// InfoQueueを設定
		void SetupInfoQueue();

	private:
		// DXGIファクトリー
		ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
		// 使用するアダプタ
		ComPtr<IDXGIAdapter4> adapter_ = nullptr;
		// Device
		ComPtr<ID3D12Device> device_ = nullptr;
		// MeshShader対応Device
		ComPtr<ID3D12Device10> device10_ = nullptr;
		// デバイスの対応機能
		DeviceCaps deviceCaps_{};
	};
}
