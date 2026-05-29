#pragma once

#include <map>
#include <string>
#include <memory>

#include "Pipeline/Factory/PipelineFactory.h"

namespace LGF {

	class Pipeline;

	/// <summary>
	/// パイプラインコンテナ
	/// </summary>
	class PipelineManager {
	public:
		PipelineManager();
		~PipelineManager() = default;

		// アセット内のJsonを読み込んでパイプラインを生成する
		void Initialize();

		void Finalize();

		Pipeline* GetPipeline(const std::string& pipelineName);

	private:
		// パイプラインファクトリー
		std::unique_ptr<PipelineFactory> factory_;
		// パイプラインリスト
		std::map<std::string, std::unique_ptr<Pipeline>> pipelines_;
	};

}