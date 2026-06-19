#pragma once

#include <cstdint>
#include <string>

#include <d3d12.h>

namespace LGF {
	struct RootParameterInfo {
		std::string name;
		uint32_t rootIndex = 0;
		D3D_SHADER_INPUT_TYPE type{};
		uint32_t bindPoint = 0;
		uint32_t space = 0;
	};
}