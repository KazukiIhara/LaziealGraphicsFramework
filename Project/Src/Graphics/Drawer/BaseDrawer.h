#pragma once

#include <array>
#include <cstdint>
#include <d3d12.h>
#include <vector>
#include <cstddef>

#include "Graphics/DirectX/FrameContext/FrameContext.h"

#include "Graphics/DirectX/ComPtr/ComPtr.h"

#include "IDrawer.h"
#include "Graphics/RenderPass/RenderPassTypes.h"

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
		void BeginFrame() override;

		/// <summary>
		/// CPUで積んだコマンドをGPUに投げる
		/// </summary>
		virtual void EndFrame()override = 0;

	protected:
		/// <summary>
		/// インスタンスの数が不正でないか確認して返す
		/// </summary>
		uint32_t CheckIntstanceCount();

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
		void CreateInstanceResources(unsigned long long resSize, void** data);

		void AddInstance(const void* data, uint32_t renderPassIndex);
		void PrepareRenderPassInstances();
		RenderPassInstanceRange GetRenderPassInstanceRange(uint32_t renderPassIndex) const;
		D3D12_GPU_VIRTUAL_ADDRESS GetDrawRangeAddress(uint32_t renderPassIndex) const;

	protected:
		// インスタンシング描画用のリソース
		FrameBufferedUploadBuffer instanceResources_;
		void** instanceDataTarget_ = nullptr;
		std::array<uint32_t, kFrameContextCount> instanceSrvIndices_{};

		// インスタンスリソースのsrvインデックス
		uint32_t instanceSrvIndex_{};

		// インスタンスのカウント
		uint32_t instanceCount_{};
		// インスタンス追加用インデックス
		uint32_t currentIndex_{};

		// インスタンス最大数
		uint32_t numMaxInstance_{};
		uint32_t instanceStride_{};
		std::vector<uint32_t> instanceRenderPassIndices_;
		std::vector<std::byte> sortedInstanceData_;
		std::array<RenderPassInstanceRange, kMaxRenderPassCount> renderPassRanges_{};
		FrameBufferedUploadBuffer drawRangeResources_;

	private:
		DirectXCore& directX_;
		PipelineManager& pipelineMgr_;

		Pipeline* pipeline_ = nullptr;
	};
}
