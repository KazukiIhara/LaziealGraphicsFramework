#pragma once

#include <memory>
#include <string>

#include <d3d12.h>

namespace LGF {

	class Device;
	class ShaderCompiler;
	class Pipeline;

	/// <summary>
	/// パイプライン生成クラス
	/// </summary>
	class PipelineFactory {
	public:
		PipelineFactory(Device& device, ShaderCompiler& shaderCompiler);
		~PipelineFactory() = default;

		std::unique_ptr<Pipeline> CreatePipeline(const std::string& jsonPath);

	private:
		Device& device_;
		ShaderCompiler& shaderCompiler_;
	};

}