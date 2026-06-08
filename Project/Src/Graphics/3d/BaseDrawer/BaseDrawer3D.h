#pragma once

#include <cstdint>
#include <string>
#include <d3d12.h>
#include "ComPtr/ComPtr.h"
#include "3d/IDrawer3D.h"

namespace LGF {

	class DirectXCore;
	class Pipeline;

	/// <summary>
	/// 3D描画基底クラス
	/// </summary>
	class BaseDrawer3D :public IDrawer3D {
	protected:
		struct InitParam {
			uint32_t numMaxInstance;
			unsigned long long instResSize;
			unsigned long long matResSize;
			void** instData;
			void** matData;
		};
	public:
		BaseDrawer3D(DirectXCore* directx);
		virtual ~BaseDrawer3D();

		/// <summary>
		/// 描画クラス固有の初期化処理
		/// </summary>
		virtual void Initialize()override = 0;

		/// <summary>
		/// CPUで積んだコマンドをGPUに投げる
		/// </summary>
		virtual void ThrowDrawCommand()override = 0;

		/// <summary>
		/// パイプラインをセット
		/// </summary>
		void SetPipeline(Pipeline* pipeline);

	protected:
		DirectXCore* GetDirectX();

		/// <summary>
		/// リソースの作成など共通の初期化処理
		/// </summary>
		void Initialize(InitParam param);

		/// <summary>
		/// インスタンスリソースを作成
		/// </summary>
		void CreateInstanceResource(unsigned long long resSize, void** data,
			ComPtr<ID3D12Resource>& res, uint32_t& srvIdx);

	protected:
		// インスタンシング描画用のリソース
		ComPtr<ID3D12Resource> instanceResource_{};
		// マテリアルのリソース
		ComPtr<ID3D12Resource> materialResource_{};

		// インスタンスリソースのsrvインデックス
		uint32_t instanceSrvIndex_{};
		// マテリアルリソースのsrvインデックス
		uint32_t materialSrvIndex_{};

		// インスタンスのカウント
		uint32_t instanceCount_{};
		// インスタンス追加用インデックス
		uint32_t currentIndex_{};

		// インスタンス最大数
		uint32_t numMaxInstance_{};

	private:
		DirectXCore* directX_;
		Pipeline* pipeline_ = nullptr;
	};

}