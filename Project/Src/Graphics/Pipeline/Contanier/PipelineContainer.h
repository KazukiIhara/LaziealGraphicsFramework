#pragma once

#include <map>
#include <string>
#include <memory>

namespace LGF {

	class BasePipeline;

	/// <summary>
	/// パイプラインコンテナ
	/// </summary>
	class PipelineStateContainer {
	public:
		PipelineStateContainer();
		~PipelineStateContainer() = default;

		void Initialize();

		void Finalize();

		BasePipeline* GetPipeline(const std::string& pipelineName);

	private:
		// パイプラインリスト
		std::map<std::string, std::unique_ptr<BasePipeline>> pipelines_;
	};

}