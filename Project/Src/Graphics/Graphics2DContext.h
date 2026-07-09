#pragma once

namespace LGF {
	class DirectXCore;
	class PipelineManager;
	class Camera2DSystem;

	struct Graphics2DContext {
		DirectXCore& directX;
		PipelineManager& pipelineMgr;
		Camera2DSystem& cameraSys;
	};
}