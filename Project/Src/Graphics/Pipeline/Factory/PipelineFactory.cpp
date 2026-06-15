#include "PipelineFactory.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/JsonLoader/PipelineJsonLoader.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

PipelineFactory::PipelineFactory(DirectXCore& directX)
	: directX_(directX) {
}

PipelineFactory::~PipelineFactory() = default;

std::unique_ptr<Pipeline> PipelineFactory::CreatePipeline(const std::string& jsonPath) {
	return nullptr;
}
