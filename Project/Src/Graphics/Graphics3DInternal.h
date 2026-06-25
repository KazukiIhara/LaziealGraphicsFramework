#pragma once

namespace LGF {

	class Camera3DSystem;
	class Renderer3D;

	namespace Graphics3D::Internal {

		void SetCameraSystem(Camera3DSystem* cameraSystem);
		void ResetCameraSystem();

		void SetRenderer(Renderer3D* renderer);
		void ResetRenderer();

	}

}