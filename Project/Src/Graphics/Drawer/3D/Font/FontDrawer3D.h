#pragma once

#include <cstdint>
#include <memory>

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"
#include "LGF/Math/MathTypes.h"

namespace LGF {

	struct FontGlyph3DForGPU {
		Vector3 origin{};
		float padding0{};
		Vector2 offset{};
		Vector2 size{};
		Vector2 uvScale{};
		Vector2 uvOffset{};
		Vector3 right{ 1.0f, 0.0f, 0.0f };
		uint32_t textureIndex = UINT32_MAX;
		Vector3 up{ 0.0f, 1.0f, 0.0f };
		uint32_t billboard{};
		Vector4 color{};
	};
	static_assert(sizeof(FontGlyph3DForGPU) == 96u);

	class FontDrawer3D : public BaseDrawer3D {
	public:
		explicit FontDrawer3D(Graphics3DContext context);

		void Initialize() override;
		void BeginFrame() override;
		void EndFrame() override;
		void AddGlyph(const FontGlyph3DForGPU& data);

	private:
		static constexpr uint32_t kPageCapacity = 4096u;
		FontGlyph3DForGPU* instanceData_ = nullptr;
		std::unique_ptr<FontDrawer3D> nextPage_;
	};

}
