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

}

void PipelineManager::Finalize() {

}

Pipeline* PipelineManager::GetPipeline(const std::string& pipelineName) {
	return nullptr;
}

void PipelineManager::LoadAll() {

}
