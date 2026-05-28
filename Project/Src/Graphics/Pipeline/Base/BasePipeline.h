#pragma once

#include <unordered_map>
#include <string>

#include <d3d12.h>

#include "ComPtr/ComPtr.h"

namespace LGF {

	/// <summary>
	/// パイプライン基底クラス
	/// </summary>
	class BasePipeline {
	public:
		virtual ~BasePipeline() = default;

		ID3D12PipelineState* GetPipelineState() const;
		ID3D12RootSignature* GetRootSignature() const;

	private:
		ComPtr<ID3D12PipelineState> pipelineState_;
		ComPtr<ID3D12RootSignature> rootSignature_;

		std::unordered_map<std::string, ComPtr<ID3DBlob>> shaderBlobs_;

	};
}

