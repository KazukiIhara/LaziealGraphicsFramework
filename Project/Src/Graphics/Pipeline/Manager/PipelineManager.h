#pragma once

#include <map>
#include <string>
#include <memory>

#include "Pipeline/Factory/PipelineFactory.h"

namespace LGF {

	class DirectXCore;
	class Pipeline;

	/// <summary>
	/// パイプラインマネージャ
	/// </summary>
	class PipelineManager {
	public:
		PipelineManager(DirectXCore& directX);
		~PipelineManager();

		void Initialize();

		void Finalize();

		Pipeline* GetPipeline(const std::string& pipelineName);

	private:
		// assetフォルダ内の全シェーダーを読む
		void LoadAll();

	private:
		// パイプラインファクトリー
		std::unique_ptr<PipelineFactory> factory_;
		// パイプラインリスト
		std::map<std::string, std::unique_ptr<Pipeline>> pipelines_;
	};

}