#include "PipelineFactory.h"

#include <cassert>
#include <vector>
#include <string>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/JsonLoader/PipelineJsonLoader.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

PipelineFactory::PipelineFactory(DirectXCore& directX)
	: directX_(directX) {
}

PipelineFactory::~PipelineFactory() = default;

std::unique_ptr<Pipeline> PipelineFactory::CreatePipeline(const std::string& jsonPath) {



	// パイプラインを生成して返す
	Pipeline::InitParam initParam;

	return std::make_unique<Pipeline>(initParam);
}
