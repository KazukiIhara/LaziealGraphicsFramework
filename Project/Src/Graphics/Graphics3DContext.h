#pragma once

namespace LGF {
	class DirectXCore;
	class PipelineManager;
	class Camera3DSystem;
	class ToonLightingSystem;

	struct Graphics3DContext {
		DirectXCore& directX;
		PipelineManager& pipelineMgr;
		Camera3DSystem& cameraSys;
		ToonLightingSystem& toonLightingSystem;
	};
}
