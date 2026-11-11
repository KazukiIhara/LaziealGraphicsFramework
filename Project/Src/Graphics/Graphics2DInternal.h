#pragma once

#include <string_view>

#include "LGF/Math/Vector2.h"
#include "LGF/Math/Vector4.h"

namespace LGF {

	class DirectXCore;
	class Camera2DSystem;
	class Renderer2D;
	class Font2D;
	struct Font2DDrawOptions;

	namespace Graphics2D::Internal {

		void SetDirectXCore(DirectXCore* directX);
		void ResetDirectXCore();

		void SetCamera2DSystem(Camera2DSystem* cameraSystem);
		void ResetCamera2DSystem();

		void SetRenderer2D(Renderer2D* renderer);
		void ResetRenderer2D();
		void DrawFont(
			const Font2D& font,
			std::string_view text,
			const Vector2& position,
			const Font2DDrawOptions& options);

	}

}
