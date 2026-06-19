#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include <d3d12.h>
#include <d3dcompiler.h>

#include "ComPtr/ComPtr.h"

#include "Pipeline/JsonLoader/PipelineJsonLoader.h"

#include "Pipeline/ShaderBlob.h"

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
		std::vector<ShaderBlob> CompileShaders(
			const PipelineJsonLoader::ShaderParam& shaderParam);

		ComPtr<ID3D12RootSignature> CreateRootSignature(
			const std::vector<ShaderBlob>& shaderBlobs);

		ComPtr<ID3D12PipelineState> CreateMeshGraphicsPipelineState(
			const PipelineJsonLoader::Param& jsonParam,
			const ComPtr<ID3D12RootSignature>& rootSignature,
			const std::vector<ShaderBlob>& shaderBlobs);

	private:
		DirectXCore& directX_;
	};

}