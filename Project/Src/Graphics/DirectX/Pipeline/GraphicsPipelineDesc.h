#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <dxgi1_6.h>

namespace LGF {

	struct ShaderDesc {
		std::wstring path;
		std::wstring entryPoint;
		std::wstring profile;
	};

	struct InputElementDesc {
		std::string semanticName;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		uint32_t slot = 0u;
		uint32_t offset = 0u;
	};

	struct GraphicsPipelineDesc {
		ShaderDesc vertexShader;
		ShaderDesc pixelShader;
		DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		std::vector<InputElementDesc> inputLayout;
	};

}
