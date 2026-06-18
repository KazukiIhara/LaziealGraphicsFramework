#include "Pipeline.h"

using namespace LGF;

Pipeline::Pipeline() {
}

Pipeline::~Pipeline() = default;

void Pipeline::Initialize(
	const std::string& name,
	const PipelineType& type,
	ComPtr<ID3D12PipelineState> pipelineState,
	ComPtr<ID3D12RootSignature> rootSignature,
	std::unordered_map<std::string, ComPtr<ID3DBlob>> shaderBlobs
) {
	name_ = name;
	type_ = type;
	pipelineState_ = pipelineState;
	rootSignature_ = rootSignature;
	shaderBlobs_ = shaderBlobs;
}

const std::string& Pipeline::GetName() const {
	return name_;
}

PipelineType Pipeline::GetType() const {
	return type_;
}

ID3D12PipelineState* Pipeline::GetPipelineState() const {
	return pipelineState_.Get();
}

ID3D12RootSignature* Pipeline::GetRootSignature() const {
	return rootSignature_.Get();
}

ID3DBlob* Pipeline::GetShaderBlob(const std::string& shaderName) const {
	const auto it = shaderBlobs_.find(shaderName);
	if (it == shaderBlobs_.end()) {
		return nullptr;
	}

	return it->second.Get();
}

bool Pipeline::IsValid() const {
	return pipelineState_ != nullptr && rootSignature_ != nullptr;
}