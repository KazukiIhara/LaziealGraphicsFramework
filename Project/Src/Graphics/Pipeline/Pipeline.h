#pragma once

#include <string>
#include <unordered_map>

#include <d3d12.h>
#include <d3dcommon.h>

#include "ComPtr/ComPtr.h"

#include "Pipeline/ShaderBlob.h"
#include "Pipeline/RootParameterInfo.h"

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
			std::vector<ShaderBlob> shaderBlobs;
			std::vector<RootParameterInfo> rootParamInfos;
		};
	public:
		Pipeline(InitParam initParam);
		~Pipeline();

		const std::string& GetName() const;

		ID3D12PipelineState* GetPipelineState() const;
		ID3D12RootSignature* GetRootSignature() const;

		ID3DBlob* GetShaderBlob(const std::string& shaderName) const;
		uint32_t GetRootIndex(const std::string& name) const;

		bool IsValid() const;

	private:
		std::string name_;

		ComPtr<ID3D12PipelineState> pipelineState_;
		ComPtr<ID3D12RootSignature> rootSignature_;
		std::vector<ShaderBlob> shaderBlobs_;
		std::vector<RootParameterInfo> rootParamInfos_;
	};
}

