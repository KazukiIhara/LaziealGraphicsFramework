#pragma once

#include <cstdint>

namespace LGF {

	constexpr uint32_t kMaxRenderPassCount = 8u;
	constexpr uint32_t kRenderPassConstantBufferStride = 256u;

	struct RenderPassInstanceRange {
		uint32_t offset = 0u;
		uint32_t count = 0u;
	};

	struct DrawRangeForGPU {
		uint32_t instanceOffset = 0u;
		uint32_t padding[3]{};
	};

}
