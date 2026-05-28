#pragma once

#include <memory>
#include <string>

#include <d3d12.h>

namespace LGF {

	class Device;
	class ShaderCompiler;
	class Pipeline;
	struct PipelineDesc;

	/// <summary>
	/// パイプライン生成クラス
	/// </summary>
	class PipelineFactory {
	public:
		PipelineFactory(Device& device, ShaderCompiler& shaderCompiler);
		~PipelineFactory() = default;

		void Initialize();

		std::unique_ptr<Pipeline> CreatePipeline(const std::string& jsonPath);

	private:
		std::unique_ptr<Pipeline> CreateGraphicsPipeline(const PipelineDesc& desc);
		std::unique_ptr<Pipeline> CreateComputePipeline(const PipelineDesc& desc);

	private:
		Device& device_;
		ShaderCompiler& shaderCompiler_;
	};

}