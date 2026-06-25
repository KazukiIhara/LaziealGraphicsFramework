#pragma once

namespace LGF {
	class DirectXCore;
	class PipelineManager;

	struct Graphics2DContext {
		DirectXCore& directX;
		PipelineManager& pipelineMgr;
	};
}