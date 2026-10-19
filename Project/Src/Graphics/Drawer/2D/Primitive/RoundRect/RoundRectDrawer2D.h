#pragma once

#include "Graphics/Drawer/2D/Base/BaseDrawer2D.h"

#include "LGF/Math/Vector2.h"

namespace LGF {

	/// <summary>
	/// GPUに送る2D角丸四角形データ
	/// </summary>
	struct RoundRect2DForGPU {
		InstanceData2D instance;
		Vector2 leftTop;
		Vector2 rightBottom;
		Vector2 radius;
	};

	/// <summary>
	/// 2D角丸四角形
	/// </summary>
	class RoundRectDrawer2D : public BaseDrawer2D {
	public:
		explicit RoundRectDrawer2D(Graphics2DContext context);
		~RoundRectDrawer2D() override;

		void Initialize() override;
		void EndFrame() override;

		void AddRoundRect(const RoundRect2DForGPU& data);

	private:
		RoundRect2DForGPU* instanceData_ = nullptr;
	};

}
