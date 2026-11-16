#pragma once

#include <filesystem>
#include <unordered_map>

#include "Graphics/PostProcess/PostProcessTypes.h"

namespace LGF {

	class PostProcessJsonLoader {
	public:
		std::unordered_map<std::string, PostEffectDefinition> LoadEffectDefinitions(const std::filesystem::path& directory) const;
		PostProcessProfileData LoadProfile(const std::filesystem::path& filePath, const std::unordered_map<std::string, PostEffectDefinition>& definitions) const;
	};

}
