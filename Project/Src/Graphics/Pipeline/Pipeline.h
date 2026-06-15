#pragma once

#include <string>
#include <unordered_map>

#include <d3d12.h>
#include <d3dcommon.h>

#include "ComPtr/ComPtr.h"

namespace LGF {
	enum class PipelineType {
		Render,
		Compute,
		PostEffect,
	};

	class PipelineFactory;

	/// <summary>
	/// パイプラインクラス
	/// </summary>
	class Pipeline {
	public:
		Pipeline();
		~Pipeline();

		void Initialize(
			const std::string& name,
			const PipelineType& type,
			ComPtr<ID3D12PipelineState> state,
			ComPtr<ID3D12RootSignature> rootSignature,
			std::unordered_map<std::string, ComPtr<ID3DBlob>> shaderBlobs);

		const std::string& GetName() const;
		PipelineType GetType() const;

		ID3D12PipelineState* GetPipelineState() const;
		ID3D12RootSignature* GetRootSignature() const;

		ID3DBlob* GetShaderBlob(const std::string& shaderName) const;

		bool IsValid() const;

	private:
		std::string name_;
		PipelineType type_ = PipelineType::Render;

		ComPtr<ID3D12PipelineState> pipelineState_;
		ComPtr<ID3D12RootSignature> rootSignature_;

		std::unordered_map<std::string, ComPtr<ID3DBlob>> shaderBlobs_;

	};
}

