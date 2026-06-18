#pragma once

#include <string>
#include <unordered_map>

#include <d3d12.h>
#include <d3dcommon.h>

#include "ComPtr/ComPtr.h"

namespace LGF {

	class PipelineFactory;

	/// <summary>
	/// パイプラインクラス
	/// </summary>
	class Pipeline {
	public:
		struct InitParam {
			std::string name;
			ComPtr<ID3D12PipelineState> pipelineState;
			ComPtr<ID3D12RootSignature> rootSignature;
			std::unordered_map<std::string, ComPtr<ID3DBlob>> shaderBlobs;
		};
	public:
		Pipeline(InitParam initParam);
		~Pipeline();

		const std::string& GetName() const;

		ID3D12PipelineState* GetPipelineState() const;
		ID3D12RootSignature* GetRootSignature() const;

		ID3DBlob* GetShaderBlob(const std::string& shaderName) const;

		bool IsValid() const;

	private:
		std::string name_;

		ComPtr<ID3D12PipelineState> pipelineState_;
		ComPtr<ID3D12RootSignature> rootSignature_;

		std::unordered_map<std::string, ComPtr<ID3DBlob>> shaderBlobs_;

	};
}

