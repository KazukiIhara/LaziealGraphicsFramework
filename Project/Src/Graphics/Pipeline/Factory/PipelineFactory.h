#pragma once

#include <memory>
#include <string>

#include <d3d12.h>

namespace LGF {

	class DirectXCore;
	class Pipeline;

	/// <summary>
	/// パイプライン生成クラス
	/// </summary>
	class PipelineFactory {
	public:
		PipelineFactory(DirectXCore& directX);
		~PipelineFactory();

		std::unique_ptr<Pipeline> CreatePipeline(const std::string& jsonPath);

	private:
		DirectXCore& directX_;
	};

}