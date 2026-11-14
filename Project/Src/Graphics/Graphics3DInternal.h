#pragma once

#include <string_view>
#include <memory>

#include "LGF/Math/Vector3.h"

namespace LGF {

	class Camera3DSystem;
	class DirectXCore;
	class DirectionalLightSystem;
	class PointLightSystem;
	class Renderer3D;
	class SkyboxSystem;
	class Font3D;
	class Texture;
	struct RenderTargetTextureDesc;
	struct RenderPassTargetResource;
	struct Font3DDrawOptions;

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
		std::shared_ptr<RenderPassTargetResource> CreateRenderTarget(
			const RenderTargetTextureDesc& desc,
			Texture& texture);
		void DrawFont(
			const Font3D& font,
			std::string_view text,
			const Vector3& position,
			const Font3DDrawOptions& options);

	}

}
