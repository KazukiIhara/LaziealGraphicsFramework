#pragma once

#include <cstdint>
#include <memory>

#include "LGF/Graphics/RenderTargetTexture.h"

namespace LGF {

	class DepthStencil;
	class RenderTexture;

	struct RenderPassTargetResource {
		std::unique_ptr<RenderTexture> output;
		std::unique_ptr<RenderTexture> sceneColor;
		std::unique_ptr<RenderTexture> gBufferAlbedo;
		std::unique_ptr<RenderTexture> gBufferNormal;
		std::unique_ptr<RenderTexture> gBufferMaterial;
		std::unique_ptr<DepthStencil> depthStencil;
		uint32_t width = 0u;
		uint32_t height = 0u;
		bool isOutputInitialized = false;
	};

}
