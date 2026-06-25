#pragma once

#include <cstdint>
#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

#include "IDrawer.h"

namespace LGF {

	class DirectXCore;
	class PipelineManager;
	class Pipeline;

	/// <summary>
	/// 汎用描画基底クラス
	/// </summary>
	class BaseDrawer :public IDrawer {
	protected:
		struct InitParam {
			uint32_t numMaxInstance;
			unsigned long long instResSize;
			void** instData;
		};
	public:
		BaseDrawer(DirectXCore& directX, PipelineManager& pipelineMgr);
		virtual ~BaseDrawer()override;

		/// <summary>
		/// 描画クラス固有の初期化処理
		/// </summary>
		virtual void Initialize()override = 0;

		/// <summary>
		/// CPUで積んだコマンドをGPUに投げる
		/// </summary>
		virtual void EndFrame()override = 0;

	protected:
		/// <summary>
		/// パイプラインを取得
		/// </summary>
		Pipeline* GetPipeline();

		/// <summary>
		/// パイプラインをセット
		/// </summary>
		void SetPipeline(Pipeline* pipeline);

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

		// インスタンスリソースのsrvインデックス
		uint32_t instanceSrvIndex_{};

		// インスタンスのカウント
		uint32_t instanceCount_{};
		// インスタンス追加用インデックス
		uint32_t currentIndex_{};

		// インスタンス最大数
		uint32_t numMaxInstance_{};

	private:
		DirectXCore& directX_;
		PipelineManager& pipelineMgr_;

		Pipeline* pipeline_ = nullptr;
	};
}