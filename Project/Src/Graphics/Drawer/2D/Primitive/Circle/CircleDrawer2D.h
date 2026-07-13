#pragma once

#include "Graphics/Drawer/2D/Base/BaseDrawer2D.h"

#include "LGF/Math/Vector2.h"

namespace LGF {

	/// <summary>
	/// GPUに送る2D円データ
	/// </summary>
	struct Circle2DForGPU {
		InstanceData2D instance;
		Vector2 center;
		Vector2 radius;
	};

	/// <summary>
	/// 2D円
	/// </summary>
	class CircleDrawer2D :public BaseDrawer2D {
	public:
		CircleDrawer2D(Graphics2DContext context);
		~CircleDrawer2D()override;

		void Initialize()override;

		void EndFrame()override;

		void AddCircle(const Circle2DForGPU& data);

	private:
		Circle2DForGPU* instanceData_ = nullptr;

	};

}
