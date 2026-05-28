#pragma once

#include <string>
#include <unordered_map>

#include <d3d12.h>

#include "ComPtr/ComPtr.h"

namespace LGF {
	enum class PipelineType {
		Graphics,
		Compute,
	};

	class PipelineFactory;

	/// <summary>
	/// パイプラインクラス
	/// </summary>
	class Pipeline {
	public:
		Pipeline();
		~Pipeline() = default;

		const std::string& GetName() const;
		PipelineType GetType() const;

		ID3D12PipelineState* GetPipelineState() const;
		ID3D12RootSignature* GetRootSignature() const;

		ID3DBlob* GetShaderBlob(const std::string& shaderName) const;

		bool IsValid() const;

	private:
		void SetName(const std::string& name);
		void SetType(PipelineType type);

		void SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState);
		void SetRootSignature(ComPtr<ID3D12RootSignature> rootSignature);
		void AddShaderBlob(const std::string& shaderName, ComPtr<ID3DBlob> shaderBlob);

	private:
		std::string name_;
		PipelineType type_ = PipelineType::Graphics;

		ComPtr<ID3D12PipelineState> pipelineState_;
		ComPtr<ID3D12RootSignature> rootSignature_;

		std::unordered_map<std::string, ComPtr<ID3DBlob>> shaderBlobs_;

		friend class PipelineFactory;
	};
}

