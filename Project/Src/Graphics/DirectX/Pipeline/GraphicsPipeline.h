#pragma once

#include <string>
#include <vector>

#include <d3d12.h>

#include "ComPtr/ComPtr.h"
#include "GraphicsPipelineDesc.h"

namespace LGF {

	class ShaderCompiler;

	class GraphicsPipeline {
	public:
		bool Initialize(
			ID3D12Device* device,
			ShaderCompiler& shaderCompiler,
			const GraphicsPipelineDesc& desc);

		ID3D12PipelineState* GetPipelineState()const;
		ID3D12RootSignature* GetRootSignature()const;

	private:
		bool CreateRootSignature(ID3D12Device* device);
		bool CreatePipelineState(
			ID3D12Device* device,
			ShaderCompiler& shaderCompiler,
			const GraphicsPipelineDesc& desc);

	private:
		ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
		ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;
		std::vector<std::string> semanticNames_;
	};

}
