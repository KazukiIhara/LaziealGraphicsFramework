#pragma once

#include <filesystem>

#include "GraphicsPipelineDesc.h"

namespace LGF {

	class PipelineJsonLoader {
	public:
		static bool Load(const std::filesystem::path& filePath, GraphicsPipelineDesc& outDesc);
	};

}
