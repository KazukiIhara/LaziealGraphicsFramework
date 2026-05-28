#include "PipelineJsonLoader.h"

#include <fstream>
#include <format>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

#include "Logger/Logger.h"

using namespace LGF;

namespace {

	std::string ReadTextFile(const std::filesystem::path& filePath) {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			return {};
		}

		std::ostringstream stream;
		stream << file.rdbuf();
		return stream.str();
	}

	std::string ExtractObject(const std::string& text, const std::string& key) {
		const std::regex pattern("\"" + key + "\"\\s*:\\s*\\{([^\\}]*)\\}");
		std::smatch match;
		if (!std::regex_search(text, match, pattern)) {
			return {};
		}

		return match[1].str();
	}

	std::string ExtractString(const std::string& text, const std::string& key) {
		const std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
		std::smatch match;
		if (!std::regex_search(text, match, pattern)) {
			return {};
		}

		return match[1].str();
	}

	uint32_t ExtractUInt(const std::string& text, const std::string& key, uint32_t defaultValue) {
		const std::regex pattern("\"" + key + "\"\\s*:\\s*(\\d+)");
		std::smatch match;
		if (!std::regex_search(text, match, pattern)) {
			return defaultValue;
		}

		return static_cast<uint32_t>(std::stoul(match[1].str()));
	}

	DXGI_FORMAT ToFormat(const std::string& value) {
		if (value == "R8G8B8A8_UNORM") {
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		if (value == "R8G8B8A8_UNORM_SRGB") {
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		}
		if (value == "D24_UNORM_S8_UINT") {
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		}
		if (value == "R32G32B32_FLOAT") {
			return DXGI_FORMAT_R32G32B32_FLOAT;
		}
		if (value == "R32G32B32A32_FLOAT") {
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	ShaderDesc ReadShaderDesc(const std::string& text, const std::string& key) {
		const std::string objectText = ExtractObject(text, key);

		ShaderDesc desc{};
		desc.path = Logger::ConvertString(ExtractString(objectText, "path"));
		desc.entryPoint = Logger::ConvertString(ExtractString(objectText, "entry"));
		desc.profile = Logger::ConvertString(ExtractString(objectText, "profile"));
		return desc;
	}

	void ReadInputLayout(const std::string& text, std::vector<InputElementDesc>& outInputLayout) {
		const std::regex arrayPattern("\"inputLayout\"\\s*:\\s*\\[([\\s\\S]*?)\\]");
		std::smatch arrayMatch;
		if (!std::regex_search(text, arrayMatch, arrayPattern)) {
			return;
		}

		const std::string arrayText = arrayMatch[1].str();
		const std::regex elementPattern("\\{([^\\}]*)\\}");

		for (std::sregex_iterator it(arrayText.begin(), arrayText.end(), elementPattern);
			it != std::sregex_iterator();
			++it) {

			const std::string elementText = (*it)[1].str();
			InputElementDesc element{};
			element.semanticName = ExtractString(elementText, "semantic");
			element.format = ToFormat(ExtractString(elementText, "format"));
			element.slot = ExtractUInt(elementText, "slot", 0u);
			element.offset = ExtractUInt(elementText, "offset", 0u);

			if (!element.semanticName.empty() && element.format != DXGI_FORMAT_UNKNOWN) {
				outInputLayout.push_back(element);
			}
		}
	}

}

bool PipelineJsonLoader::Load(const std::filesystem::path& filePath, GraphicsPipelineDesc& outDesc) {
	const std::string text = ReadTextFile(filePath);
	if (text.empty()) {
		Logger::Log(std::format("Failed to read pipeline json: {}\n", filePath.string()));
		return false;
	}

	GraphicsPipelineDesc desc{};
	desc.vertexShader = ReadShaderDesc(text, "vertexShader");
	desc.pixelShader = ReadShaderDesc(text, "pixelShader");
	desc.renderTargetFormat = ToFormat(ExtractString(text, "renderTargetFormat"));
	desc.depthStencilFormat = ToFormat(ExtractString(text, "depthStencilFormat"));
	ReadInputLayout(text, desc.inputLayout);

	if (desc.vertexShader.path.empty() ||
		desc.vertexShader.entryPoint.empty() ||
		desc.vertexShader.profile.empty() ||
		desc.pixelShader.path.empty() ||
		desc.pixelShader.entryPoint.empty() ||
		desc.pixelShader.profile.empty() ||
		desc.renderTargetFormat == DXGI_FORMAT_UNKNOWN ||
		desc.depthStencilFormat == DXGI_FORMAT_UNKNOWN ||
		desc.inputLayout.empty()) {

		Logger::Log(std::format("Invalid pipeline json: {}\n", filePath.string()));
		return false;
	}

	outDesc = std::move(desc);
	return true;
}
