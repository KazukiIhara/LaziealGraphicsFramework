#pragma once

#include <map>
#include <string>
#include <memory>

namespace LGF {

	class Pipeline;

	/// <summary>
	/// パイプラインコンテナ
	/// </summary>
	class PipelineStateContainer {
	public:
		PipelineStateContainer();
		~PipelineStateContainer() = default;

		void Initialize();

		void Finalize();

		Pipeline* GetPipeline(const std::string& pipelineName);

	private:
		// パイプラインリスト
		std::map<std::string, std::unique_ptr<Pipeline>> pipelines_;
	};

}