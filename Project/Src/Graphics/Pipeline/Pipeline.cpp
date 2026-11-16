#include "Pipeline.h"

#include <cassert>
#include <utility>

using namespace LGF;

Pipeline::Pipeline(InitParam initParam) {
	name_ = std::move(initParam.name);
	pipelineState_ = std::move(initParam.pipelineState);
	rootSignature_ = std::move(initParam.rootSignature);
	shaderBlobs_ = std::move(initParam.shaderBlobs);
	rootParamInfos_ = std::move(initParam.rootParamInfos);
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
	for (const ShaderBlob& shaderBlob : shaderBlobs_) {
		if (shaderBlob.stageName == shaderName) {
			return shaderBlob.blob.Get();
		}
	}

	return nullptr;
}

uint32_t Pipeline::GetRootIndex(const std::string& name) const {
	const std::optional<uint32_t> rootIndex = FindRootIndex(name);
	if (rootIndex) {
		return *rootIndex;
	}

	assert(false);
	return 0;
}

std::optional<uint32_t> Pipeline::FindRootIndex(std::string_view name) const {
	for (const RootParameterInfo& info : rootParamInfos_) {
		if (info.name == name) {
			return info.rootIndex;
		}
	}

	return std::nullopt;
}

bool Pipeline::IsValid() const {
	return pipelineState_ != nullptr && rootSignature_ != nullptr;
}
