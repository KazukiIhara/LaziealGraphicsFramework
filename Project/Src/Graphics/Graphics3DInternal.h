#pragma once

namespace LGF {

	class Camera3DSystem;
	class DirectXCore;
	class DirectionalLightSystem;
	class Renderer3D;
	class WindowSystem;

	namespace Graphics3D::Internal {
		void SetDirectXCore(DirectXCore* directX);
		void ResetDirectXCore();

		void SetWindowSystem(WindowSystem* windowSystem);
		void ResetWindowSystem();

		void SetCamera3DSystem(Camera3DSystem* cameraSystem);
		void ResetCamera3DSystem();

		void SetDirectionalLightSystem(DirectionalLightSystem* lightSystem);
		void ResetDirectionalLightSystem();

		void SetRenderer3D(Renderer3D* renderer);
		void ResetRenderer3D();

	}

}
