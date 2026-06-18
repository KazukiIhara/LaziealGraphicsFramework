#include "PipelineJsonLoader.h"

#include <fstream>
#include <cassert>

#include <nlohmann/json.hpp>

using namespace LGF;

PipelineJsonLoader::PipelineJsonLoader() {
}

PipelineJsonLoader::~PipelineJsonLoader() = default;

PipelineJsonLoader::Param PipelineJsonLoader::Load(const std::string& filePath) {
	// Jsonファイルを読み込む
	const std::string jsonText = LoadTextFile(filePath);

	// Json文字列を解析する
	const nlohmann::json root = nlohmann::json::parse(jsonText);

	Param param{};

	// 名前
	param.name = root.value("Name", "");

	// シェーダー情報
	if (root.contains("Shaders")) {
		const auto& shaders = root["Shaders"];

		param.shaders.amplification = shaders.value("AS", "");
		param.shaders.mesh = shaders.value("MS", "");
		param.shaders.vertex = shaders.value("VS", "");
		param.shaders.pixel = shaders.value("PS", "");
		param.shaders.compute = shaders.value("CS", "");
	}

	// ラスタライザ設定
	if (root.contains("Rasterizer")) {
		const auto& rasterizer = root["Rasterizer"];

		param.rasterizer.cullMode = rasterizer.value("CullMode", "Back");
		param.rasterizer.fillMode = rasterizer.value("FillMode", "Solid");
	}

	// ブレンド設定
	if (root.contains("Blend")) {
		const auto& blend = root["Blend"];

		param.blend.enable = blend.value("Enable", false);
	}

	// 深度設定
	if (root.contains("Depth")) {
		const auto& depth = root["Depth"];

		param.depth.enable = depth.value("Enable", true);
		param.depth.write = depth.value("Write", true);
		param.depth.func = depth.value("Func", "LessEqual");
	}

	// レンダーターゲット設定
	if (root.contains("RenderTarget")) {
		const auto& renderTarget = root["RenderTarget"];

		// フォーマット一覧を読み込む
		if (renderTarget.contains("Formats")) {
			param.renderTarget.formats =
				renderTarget["Formats"].get<std::vector<std::string>>();
		}
	}

	// 深度ステンシル設定
	if (root.contains("DepthStencil")) {
		const auto& depthStencil = root["DepthStencil"];

		param.depthStencil.format =
			depthStencil.value("Format", "D24_UNORM_S8_UINT");
	}

	// プリミティブ設定
	param.primitiveTopology = root.value("PrimitiveTopology", "Triangle");

	assert(!param.name.empty());

	return param;
}

std::string PipelineJsonLoader::LoadTextFile(const std::string& filePath) {
	std::ifstream file(filePath);
	assert(file.is_open());

	// ファイル内容を読み込む
	std::string text;
	std::string line;
	while (std::getline(file, line)) {
		text += line;
		text += '\n';
	}

	return text;
}