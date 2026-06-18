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
	// 名前を取得
	const std::string name;
	// タイプを取得
	const PipelineType type;

	// シェーダーのマップを作成
	std::unordered_map<std::string, ComPtr<ID3DBlob>> shaderBlobs;


	// パイプラインを生成して返す

	return std::make_unique<Pipeline>();
}
