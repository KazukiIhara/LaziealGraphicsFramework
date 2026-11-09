#include "PipelineJsonLoader.h"

#include <fstream>
#include <cassert>

using namespace LGF;

namespace {

	std::string ReadString(
		const nlohmann::json& json,
		const std::string& key,
		const std::string& defaultValue) {

		return json.value(key, defaultValue);
	}

	std::filesystem::path ReadPath(const nlohmann::json& json, const std::string& key) {
		const std::string value = ReadString(json, key, "");
		return std::filesystem::path(std::u8string(value.begin(), value.end()));
	}

	bool ReadBool(
		const nlohmann::json& json,
		const std::string& key,
		bool defaultValue) {

		return json.value(key, defaultValue);
	}

	std::vector<std::string> ReadStringArray(
		const nlohmann::json& json,
		const std::string& key,
		const std::vector<std::string>& defaultValue) {

		if (!json.contains(key)) {
			return defaultValue;
		}

		return json[key].get<std::vector<std::string>>();
	}
}

PipelineJsonLoader::PipelineJsonLoader() {
}

PipelineJsonLoader::~PipelineJsonLoader() = default;

PipelineJsonLoader::Param PipelineJsonLoader::Load(const std::filesystem::path& filePath) {
	// Jsonファイルを読み込む
	const nlohmann::json root = LoadJson(filePath);

	Param param{};

	// 基本情報
	param.name = ReadString(root, "Name", "");
	param.pipelineKind = ReadString(root, "PipelineKind", "MeshGraphics");

	// シェーダー情報
	if (root.contains("Shaders")) {
		const auto& shaders = root["Shaders"];

		param.shaders.amplification = ReadPath(shaders, "AS");
		param.shaders.mesh = ReadPath(shaders, "MS");
		param.shaders.vertex = ReadPath(shaders, "VS");
		param.shaders.pixel = ReadPath(shaders, "PS");
		param.shaders.compute = ReadPath(shaders, "CS");
	}

	// ラスタライザ設定
	if (root.contains("Rasterizer")) {
		const auto& rasterizer = root["Rasterizer"];

		param.rasterizer.cullMode = ReadString(rasterizer, "CullMode", "Back");
		param.rasterizer.fillMode = ReadString(rasterizer, "FillMode", "Solid");
	}

	// ブレンド設定
	if (root.contains("Blend")) {
		const auto& blend = root["Blend"];

		param.blend.mode = ReadString(blend, "Mode", "None");
	}

	// 深度設定
	if (root.contains("Depth")) {
		const auto& depth = root["Depth"];

		param.depth.enable = ReadBool(depth, "Enable", true);
		param.depth.write = ReadBool(depth, "Write", true);
		param.depth.func = ReadString(depth, "Func", "LessEqual");
	}

	// レンダーターゲット設定
	if (root.contains("RenderTarget")) {
		const auto& renderTarget = root["RenderTarget"];

		param.renderTarget.formats = ReadStringArray(
			renderTarget,
			"Formats",
			{ "R8G8B8A8_UNORM_SRGB" });
	}

	// 深度ステンシル設定
	if (root.contains("DepthStencil")) {
		const auto& depthStencil = root["DepthStencil"];

		param.depthStencil.format = ReadString(
			depthStencil,
			"Format",
			"D24_UNORM_S8_UINT");
	}

	// プリミティブ設定
	param.primitiveTopology = ReadString(root, "PrimitiveTopology", "Triangle");

	assert(!param.name.empty());

	return param;
}

nlohmann::json PipelineJsonLoader::LoadJson(const std::filesystem::path& filePath) {
	// Json文字列を解析する
	return nlohmann::json::parse(LoadTextFile(filePath));
}

std::string PipelineJsonLoader::LoadTextFile(const std::filesystem::path& filePath) {
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
