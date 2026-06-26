#pragma once

#include "Graphics/Drawer/2D/Base/BaseDrawer2D.h"


namespace LGF {

	/// <summary>
	/// 2D三角形
	/// </summary>
	class TriangleDrawer2D :public BaseDrawer2D {
	public:
		TriangleDrawer2D(Graphics2DContext context);
		~TriangleDrawer2D()override;

		void Initialize()override;

		void EndFrame()override;

	private:

	};

}