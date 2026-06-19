#include "PipelineManager.h"

#include <cassert>
#include <filesystem>

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
	namespace fs = std::filesystem;

	// パイプラインフォルダのディレクトリパス
	constexpr char kPipelineDir[] = "Assets/JsonData/Pipeline";

	// パイプラインフォルダ内を全走査
	for (const auto& entry : fs::recursive_directory_iterator(kPipelineDir)) {

		// json拡張子でなければスキップ
		if (entry.path().extension() != ".json") {
			assert(false);
			continue;
		}

		// パイプラインを作成
		auto pipeline = factory_->CreatePipeline(entry.path().string());

		// パイプラインをマップにセット
		pipelines_.insert({ pipeline->GetName(), std::move(pipeline) });

	}
}
