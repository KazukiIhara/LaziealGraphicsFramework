#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <d3d12.h>
#include <d3dcompiler.h>

#include "ComPtr/ComPtr.h"

#include "Pipeline/JsonLoader/PipelineJsonLoader.h"

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
		std::unordered_map<std::string, ComPtr<ID3DBlob>> CompileShaders(
			const PipelineJsonLoader::ShaderParam& shaderParam);

		ComPtr<ID3D12RootSignature> CreateRootSignature(
			const std::unordered_map<std::string, ComPtr<ID3DBlob>>& shaderBlobs);

		ComPtr<ID3D12PipelineState> CreatePipelineState(
			const PipelineJsonLoader::Param& jsonParam,
			const ComPtr<ID3D12RootSignature>& rootSignature,
			const std::unordered_map<std::string, ComPtr<ID3DBlob>>& shaderBlobs);

	private:
		DirectXCore& directX_;
	};

}