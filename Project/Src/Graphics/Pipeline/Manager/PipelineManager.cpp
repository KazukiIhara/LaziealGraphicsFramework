#include "PipelineManager.h"

#include "Pipeline/Pipeline.h"
#include "DirectX/Core/DirectXCore.h"

using namespace LGF;

PipelineManager::PipelineManager(DirectXCore& directX) {
	factory_ = std::make_unique<PipelineFactory>(directX);
}

PipelineManager::~PipelineManager() = default;

void PipelineManager::Initialize() {
	// アセット内のJsonを読み込んでパイプラインを生成する
	LoadAll();
}

void PipelineManager::Finalize() {

}

Pipeline* PipelineManager::GetPipeline(const std::string& pipelineName) {
	auto it = pipelines_.find(pipelineName);
	if (it == pipelines_.end()) {
		return nullptr;
	}

	return it->second.get();
}

void PipelineManager::LoadAll() {

}
