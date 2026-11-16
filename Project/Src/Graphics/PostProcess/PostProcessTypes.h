#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "LGF/Graphics/PostProcess.h"
#include "LGF/Math/Vector2.h"
#include "LGF/Math/Vector3.h"
#include "LGF/Math/Vector4.h"

namespace LGF {

	inline constexpr uint32_t kPostProcessParameterSlotCount = 8u;

	enum class PostProcessStage : uint8_t {
		BeforeToneMapping,
		AfterToneMapping,
	};

	using PostProcessParameterValue = std::variant<float, Vector2, Vector3, Vector4>;

	struct PostProcessParameterDefinition {
		uint32_t slot = 0u;
		uint32_t componentOffset = 0u;
		uint32_t componentCount = 0u;
		Vector4 defaultValue{};
		std::optional<float> minimum;
		std::optional<float> maximum;
	};

	struct PostEffectDefinition {
		std::string name;
		std::string executor;
		std::string pipeline;
		std::unordered_map<std::string, std::string> pipelines;
		PostProcessStage stage = PostProcessStage::AfterToneMapping;
		float resolutionScale = 1.0f;
		std::unordered_map<std::string, PostProcessParameterDefinition> parameters;
	};

	struct ToneMappingDefinition {
		std::string name;
		std::string pipeline;
		std::unordered_map<std::string, PostProcessParameterDefinition> parameters;
	};

	struct PostProcessProfileEffectData {
		std::string id;
		std::string effect;
		bool enabled = true;
		std::unordered_map<std::string, PostProcessParameterValue> parameters;
	};

	struct PostProcessProfileData {
		struct ToneMappingData {
			std::string method;
			std::unordered_map<std::string, PostProcessParameterValue> parameters;
		};

		std::string name;
		ToneMappingData toneMapping;
		std::vector<PostProcessProfileEffectData> effects;
	};

	struct PostProcessParametersForGPU {
		std::array<Vector4, kPostProcessParameterSlotCount> parameters{};
	};
	static_assert(sizeof(PostProcessParametersForGPU) == 128u);

	struct PostProcessContextForGPU {
		Vector2 outputSize{};
		Vector2 inverseOutputSize{};
		Vector2 inputSize{};
		Vector2 inverseInputSize{};
		Vector2 sceneSize{};
		Vector2 inverseSceneSize{};
		uint32_t frameIndex{};
		float padding[3]{};
	};
	static_assert(sizeof(PostProcessContextForGPU) == 64u);

}
