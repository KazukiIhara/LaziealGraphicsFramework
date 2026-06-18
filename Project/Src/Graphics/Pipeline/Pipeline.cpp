#include "Pipeline.h"

using namespace LGF;

Pipeline::Pipeline(InitParam initParam) {
	name_ = initParam.name;
	pipelineState_ = initParam.pipelineState;
	rootSignature_ = initParam.rootSignature;
	shaderBlobs_ = initParam.shaderBlobs;
}

Pipeline::~Pipeline() = default;

const std::string& Pipeline::GetName() const {
	return name_;
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