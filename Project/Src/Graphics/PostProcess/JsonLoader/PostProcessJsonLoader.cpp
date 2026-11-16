#include "PostProcessJsonLoader.h"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>

#include <nlohmann/json.hpp>

using namespace LGF;

namespace {

	using Json = nlohmann::json;

	Json LoadJson(const std::filesystem::path& filePath) {
		std::ifstream file(filePath);
		if (!file) {
			throw std::runtime_error("Failed to open Post Process Json: " + filePath.string());
		}

		Json json;
		file >> json;
		return json;
	}

	PostProcessStage ToStage(const std::string& stage) {
		if (stage == "BeforeToneMapping") {
			return PostProcessStage::BeforeToneMapping;
		}
		if (stage == "AfterToneMapping") {
			return PostProcessStage::AfterToneMapping;
		}
		throw std::runtime_error("Unknown Post Process stage: " + stage);
	}

	std::pair<uint32_t, uint32_t> ToComponentRange(const std::string& components) {
		if (components == "X") {
			return { 0u, 1u };
		}
		if (components == "Y") {
			return { 1u, 1u };
		}
		if (components == "Z") {
			return { 2u, 1u };
		}
		if (components == "W") {
			return { 3u, 1u };
		}
		if (components == "XY") {
			return { 0u, 2u };
		}
		if (components == "YZ") {
			return { 1u, 2u };
		}
		if (components == "ZW") {
			return { 2u, 2u };
		}
		if (components == "XYZ") {
			return { 0u, 3u };
		}
		if (components == "YZW") {
			return { 1u, 3u };
		}
		if (components == "XYZW") {
			return { 0u, 4u };
		}
		throw std::runtime_error("Unsupported Post Process parameter components: " + components);
	}

	Vector4 ReadValue(const Json& json, uint32_t componentCount) {
		Vector4 result{};
		float* values[] = { &result.x, &result.y, &result.z, &result.w };
		if (componentCount == 1u) {
			if (!json.is_number()) {
				throw std::runtime_error("Scalar Post Process parameter must be a number.");
			}
			result.x = json.get<float>();
		} else {
			if (!json.is_array() || json.size() != componentCount) {
				throw std::runtime_error("Post Process parameter array size does not match Components.");
			}
			for (uint32_t index = 0u; index < componentCount; ++index) {
				*values[index] = json[index].get<float>();
			}
		}

		for (uint32_t index = 0u; index < componentCount; ++index) {
			if (!std::isfinite(*values[index])) {
				throw std::runtime_error("Post Process parameter must be finite.");
			}
		}
		return result;
	}

	PostProcessParameterValue ToParameterValue(const Json& json, uint32_t componentCount) {
		const Vector4 value = ReadValue(json, componentCount);
		switch (componentCount) {
			case 1u:
				return value.x;
			case 2u:
				return Vector2{ value.x, value.y };
			case 3u:
				return Vector3{ value.x, value.y, value.z };
			case 4u:
				return value;
			default:
				throw std::runtime_error("Unsupported Post Process parameter component count.");
		}
	}

	void ValidateParameterRange(const float* values, const PostProcessParameterDefinition& definition, const std::string& name) {
		for (uint32_t index = 0u; index < definition.componentCount; ++index) {
			if ((definition.minimum && values[index] < *definition.minimum) ||
				(definition.maximum && values[index] > *definition.maximum)) {
				throw std::runtime_error("Post Process parameter is outside its range: " + name);
			}
		}
	}

	std::unordered_map<std::string, PostProcessParameterDefinition> ReadParameterDefinitions(const Json& root) {
		std::unordered_map<std::string, PostProcessParameterDefinition> parameters;
		std::array<uint8_t, kPostProcessParameterSlotCount> usedComponents{};
		if (!root.contains("Parameters")) {
			return parameters;
		}

		for (const auto& [name, json] : root["Parameters"].items()) {
			PostProcessParameterDefinition parameter{};
			parameter.slot = json.value("Slot", kPostProcessParameterSlotCount);
			const auto [componentOffset, componentCount] = ToComponentRange(json.value("Components", ""));
			parameter.componentOffset = componentOffset;
			parameter.componentCount = componentCount;
			if (parameter.slot >= kPostProcessParameterSlotCount) {
				throw std::runtime_error("Post Process parameter Slot is out of range: " + name);
			}

			const uint8_t componentMask = static_cast<uint8_t>(((1u << componentCount) - 1u) << componentOffset);
			if ((usedComponents[parameter.slot] & componentMask) != 0u) {
				throw std::runtime_error("Post Process parameter components overlap: " + name);
			}
			usedComponents[parameter.slot] |= componentMask;

			if (!json.contains("Default")) {
				throw std::runtime_error("Post Process parameter Default is required: " + name);
			}
			parameter.defaultValue = ReadValue(json["Default"], componentCount);
			if (json.contains("Minimum")) {
				parameter.minimum = json["Minimum"].get<float>();
			}
			if (json.contains("Maximum")) {
				parameter.maximum = json["Maximum"].get<float>();
			}
			if ((parameter.minimum && !std::isfinite(*parameter.minimum)) ||
				(parameter.maximum && !std::isfinite(*parameter.maximum)) ||
				(parameter.minimum && parameter.maximum && *parameter.minimum > *parameter.maximum)) {
				throw std::runtime_error("Post Process parameter range is invalid: " + name);
			}
			ValidateParameterRange(&parameter.defaultValue.x, parameter, name + ".Default");
			parameters.emplace(name, parameter);
		}
		return parameters;
	}

	std::unordered_map<std::string, PostProcessParameterValue> ReadParameterValues(const Json& root, const std::unordered_map<std::string, PostProcessParameterDefinition>& definitions, const std::string& ownerName) {
		std::unordered_map<std::string, PostProcessParameterValue> parameters;
		if (!root.contains("Parameters")) {
			return parameters;
		}

		for (const auto& [name, value] : root["Parameters"].items()) {
			const auto definitionIterator = definitions.find(name);
			if (definitionIterator == definitions.end()) {
				throw std::runtime_error("Post Process parameter was not found: " + ownerName + "." + name);
			}
			const PostProcessParameterDefinition& definition = definitionIterator->second;
			const PostProcessParameterValue parameterValue = ToParameterValue(value, definition.componentCount);
			Vector4 storage{};
			std::visit([&storage](const auto& current) {
				using T = std::decay_t<decltype(current)>;
				if constexpr (std::is_same_v<T, float>) {
					storage.x = current;
				} else if constexpr (std::is_same_v<T, Vector2>) {
					storage = { current.x, current.y, 0.0f, 0.0f };
				} else if constexpr (std::is_same_v<T, Vector3>) {
					storage = { current.x, current.y, current.z, 0.0f };
				} else {
					storage = current;
				}
			}, parameterValue);
			ValidateParameterRange(&storage.x, definition, ownerName + "." + name);
			parameters.emplace(name, parameterValue);
		}
		return parameters;
	}

	PostEffectDefinition ReadEffectDefinition(const Json& root) {
		PostEffectDefinition definition{};
		definition.name = root.value("Name", "");
		definition.executor = root.value("Executor", "Fullscreen");
		definition.pipeline = root.value("Pipeline", "");
		definition.stage = ToStage(root.value("Stage", "AfterToneMapping"));
		definition.resolutionScale = root.value("ResolutionScale", 1.0f);

		if (definition.name.empty()) {
			throw std::runtime_error("Post Effect Name must not be empty.");
		}
		if (!std::isfinite(definition.resolutionScale) || definition.resolutionScale <= 0.0f || definition.resolutionScale > 1.0f) {
			throw std::runtime_error("Post Effect ResolutionScale must be greater than 0 and at most 1.");
		}

		if (root.contains("Pipelines")) {
			for (const auto& [name, value] : root["Pipelines"].items()) {
				definition.pipelines.emplace(name, value.get<std::string>());
			}
		}
		definition.parameters = ReadParameterDefinitions(root);
		return definition;
	}

	ToneMappingDefinition ReadToneMappingDefinition(const Json& root) {
		ToneMappingDefinition definition{};
		definition.name = root.value("Name", "");
		definition.pipeline = root.value("Pipeline", "");
		if (definition.name.empty() || definition.pipeline.empty()) {
			throw std::runtime_error("Tone Mapping definition requires Name and Pipeline.");
		}
		definition.parameters = ReadParameterDefinitions(root);
		return definition;
	}

}

std::unordered_map<std::string, PostEffectDefinition> PostProcessJsonLoader::LoadEffectDefinitions(const std::filesystem::path& directory) const {
	if (!std::filesystem::is_directory(directory)) {
		throw std::runtime_error("Post Effect directory was not found: " + directory.string());
	}

	std::unordered_map<std::string, PostEffectDefinition> definitions;
	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory)) {
		if (!entry.is_regular_file() || entry.path().extension() != ".json") {
			continue;
		}

		PostEffectDefinition definition = ReadEffectDefinition(LoadJson(entry.path()));
		if (definitions.contains(definition.name)) {
			throw std::runtime_error("Duplicate Post Effect Name: " + definition.name);
		}
		definitions.emplace(definition.name, std::move(definition));
	}
	return definitions;
}

std::unordered_map<std::string, ToneMappingDefinition> PostProcessJsonLoader::LoadToneMappingDefinitions(const std::filesystem::path& directory) const {
	if (!std::filesystem::is_directory(directory)) {
		throw std::runtime_error("Tone Mapping directory was not found: " + directory.string());
	}

	std::unordered_map<std::string, ToneMappingDefinition> definitions;
	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory)) {
		if (!entry.is_regular_file() || entry.path().extension() != ".json") {
			continue;
		}

		ToneMappingDefinition definition = ReadToneMappingDefinition(LoadJson(entry.path()));
		if (definitions.contains(definition.name)) {
			throw std::runtime_error("Duplicate Tone Mapping Name: " + definition.name);
		}
		definitions.emplace(definition.name, std::move(definition));
	}
	return definitions;
}

PostProcessProfileData PostProcessJsonLoader::LoadProfile(const std::filesystem::path& filePath, const std::unordered_map<std::string, PostEffectDefinition>& effectDefinitions, const std::unordered_map<std::string, ToneMappingDefinition>& toneMappingDefinitions) const {
	const Json root = LoadJson(filePath);
	PostProcessProfileData profile{};
	profile.name = root.value("Name", "");
	if (profile.name.empty() || !root.contains("ToneMapping") || !root["ToneMapping"].is_object() ||
		!root.contains("Effects") || !root["Effects"].is_array()) {
		throw std::runtime_error("Post Process Profile requires Name, ToneMapping, and Effects.");
	}
	const Json& toneMappingJson = root["ToneMapping"];
	profile.toneMapping.method = toneMappingJson.value("Method", "");
	const auto toneMappingIterator = toneMappingDefinitions.find(profile.toneMapping.method);
	if (toneMappingIterator == toneMappingDefinitions.end()) {
		throw std::runtime_error("Tone Mapping method was not found: " + profile.toneMapping.method);
	}
	profile.toneMapping.parameters = ReadParameterValues(toneMappingJson, toneMappingIterator->second.parameters, "ToneMapping");

	std::unordered_set<std::string> ids;
	PostProcessStage previousStage = PostProcessStage::BeforeToneMapping;
	for (const Json& json : root["Effects"]) {
		PostProcessProfileEffectData effect{};
		effect.id = json.value("Id", "");
		effect.effect = json.value("Effect", "");
		effect.enabled = json.value("Enabled", true);
		if (effect.id.empty() || !ids.emplace(effect.id).second) {
			throw std::runtime_error("Post Process Profile Effect Id must be unique and non-empty.");
		}

		const auto definitionIterator = effectDefinitions.find(effect.effect);
		if (definitionIterator == effectDefinitions.end()) {
			throw std::runtime_error("Post Effect was not found: " + effect.effect);
		}
		const PostEffectDefinition& definition = definitionIterator->second;
		if (static_cast<uint8_t>(definition.stage) < static_cast<uint8_t>(previousStage)) {
			throw std::runtime_error("Post Process Profile Effects must be ordered by Stage.");
		}
		previousStage = definition.stage;
		effect.parameters = ReadParameterValues(json, definition.parameters, effect.id);

		profile.effects.push_back(std::move(effect));
	}
	return profile;
}
