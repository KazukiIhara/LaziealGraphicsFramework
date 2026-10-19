#pragma once

#include "Graphics/Drawer/2D/Base/BaseDrawer2D.h"

#include "LGF/Math/Vector2.h"

namespace LGF {

	/// <summary>
	/// GPUに送る2D四角形データ
	/// </summary>
	struct Rect2DForGPU {
		InstanceData2D instance;
		Vector2 vertices[4];
	};

	/// <summary>
	/// 2D四角形
	/// </summary>
	class RectDrawer2D : public BaseDrawer2D {
	public:
		explicit RectDrawer2D(Graphics2DContext context);
		~RectDrawer2D() override;

		void Initialize() override;
		void EndFrame() override;

		void AddRect(const Rect2DForGPU& data);

	private:
		Rect2DForGPU* instanceData_ = nullptr;
	};

}
