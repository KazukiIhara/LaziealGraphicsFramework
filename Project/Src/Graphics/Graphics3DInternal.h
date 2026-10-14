#pragma once

namespace LGF {

	class Camera3DSystem;
	class DirectXCore;
	class DirectionalLightSystem;
	class PointLightSystem;
	class Renderer3D;
	class SkyboxSystem;

	namespace Graphics3D::Internal {
		void SetDirectXCore(DirectXCore* directX);
		void ResetDirectXCore();

		void SetCamera3DSystem(Camera3DSystem* cameraSystem);
		void ResetCamera3DSystem();

		void SetDirectionalLightSystem(DirectionalLightSystem* lightSystem);
		void ResetDirectionalLightSystem();

		void SetPointLightSystem(PointLightSystem* lightSystem);
		void ResetPointLightSystem();

		void SetSkyboxSystem(SkyboxSystem* skyboxSystem);
		void ResetSkyboxSystem();

		void SetRenderer3D(Renderer3D* renderer);
		void ResetRenderer3D();

	}

}
