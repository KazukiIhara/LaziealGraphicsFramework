#pragma once

#include <string>
#include <memory>

namespace LGF {

	class Pipeline;

	/// <summary>
	/// パイプライン生成クラス
	/// </summary>
	class PipelineFactory {
	public:
		std::unique_ptr<Pipeline> CreatePipeline(const std::string& jsonPath);
	};

}