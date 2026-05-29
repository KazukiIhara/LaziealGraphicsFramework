#include "PipelineFactory.h"

#include <cassert>

#include "Graphics/DirectX/Device/Device.h"
#include "Graphics/DirectX/ShaderCompiler/ShaderCompiler.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/Pipeline/PipelineDesc.h"
#include "Graphics/Pipeline/JsonLoader/PipelineJsonLoader.h"

using namespace LGF;

PipelineFactory::PipelineFactory(Device& device, ShaderCompiler& shaderCompiler)
	: device_(device), shaderCompiler_(shaderCompiler) {
}

void PipelineFactory::Initialize() {
}

std::unique_ptr<Pipeline> PipelineFactory::CreatePipeline(const std::string& jsonPath) {
	return nullptr;
}

std::unique_ptr<Pipeline> PipelineFactory::CreateGraphicsPipeline(const PipelineDesc& desc) {
	return nullptr;
}

std::unique_ptr<Pipeline> PipelineFactory::CreateComputePipeline(const PipelineDesc& desc) {
	return nullptr;
}
