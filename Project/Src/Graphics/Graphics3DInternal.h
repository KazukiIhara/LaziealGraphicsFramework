#pragma once

namespace LGF {

	class Camera3DSystem;
	class Renderer3D;
	class WindowSystem;

	namespace Graphics3D::Internal {

		void SetWindowSystem(WindowSystem* windowSystem);
		void ResetWindowSystem();

		void SetCamera3DSystem(Camera3DSystem* cameraSystem);
		void ResetCamera3DSystem();

		void SetRenderer3D(Renderer3D* renderer);
		void ResetRenderer3D();

	}

}
