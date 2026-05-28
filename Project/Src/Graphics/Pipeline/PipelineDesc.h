#pragma once

#include <string>
#include <vector>

#include "Graphics/Pipeline/Pipeline.h"

namespace LGF {

	struct ShaderDesc {
		std::string name;
		std::wstring path;
		std::wstring entryPoint;
		std::wstring profile;
	};

	struct PipelineDesc {
		std::string name;
		PipelineType type;

		std::vector<ShaderDesc> shaders;

	};

}