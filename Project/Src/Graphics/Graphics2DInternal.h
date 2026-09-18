#pragma once

namespace LGF {

	class DirectXCore;
	class Camera2DSystem;
	class Renderer2D;

	namespace Graphics2D::Internal {

		void SetDirectXCore(DirectXCore* directX);
		void ResetDirectXCore();

		void SetCamera2DSystem(Camera2DSystem* cameraSystem);
		void ResetCamera2DSystem();

		void SetRenderer2D(Renderer2D* renderer);
		void ResetRenderer2D();

	}

}
