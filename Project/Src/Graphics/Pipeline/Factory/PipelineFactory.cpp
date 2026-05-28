#include "PipelineFactory.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

std::unique_ptr<Pipeline> PipelineFactory::CreatePipeline(const std::string& jsonPath) {
	return std::unique_ptr<Pipeline>();
}