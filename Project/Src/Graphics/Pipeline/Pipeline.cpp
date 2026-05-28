#include "Pipeline.h"

using namespace LGF;

Pipeline::Pipeline() {
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

void Pipeline::SetName(const std::string& name) {
	name_ = name;
}

void Pipeline::SetType(PipelineType type) {
	type_ = type;
}

void Pipeline::SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState) {
	pipelineState_ = pipelineState;
}

void Pipeline::SetRootSignature(ComPtr<ID3D12RootSignature> rootSignature) {
	rootSignature_ = rootSignature;
}

void Pipeline::AddShaderBlob(const std::string& shaderName, ComPtr<ID3DBlob> shaderBlob) {
	shaderBlobs_[shaderName] = shaderBlob;
}