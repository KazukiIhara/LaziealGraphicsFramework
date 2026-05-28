#pragma once

#include <filesystem>

#include <d3d12.h>

#include "ComPtr/ComPtr.h"
#include "Pipeline/GraphicsPipeline.h"

namespace LGF {

	class ShaderCompiler;

	class TriangleRenderer {
	public:
		bool Initialize(
			ID3D12Device* device,
			ShaderCompiler& shaderCompiler,
			const std::filesystem::path& pipelineJsonPath);
		void Draw(ID3D12GraphicsCommandList* commandList);

	private:
		bool CreateVertexBuffer(ID3D12Device* device);

	private:
		GraphicsPipeline pipeline_;
		ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	};

}
