#pragma once

#include "Graphics/Drawer/2D/Base/BaseDrawer2D.h"

#include "LGF/Math/Vector2.h"

namespace LGF {

	/// <summary>
	/// GPUに送る2D三角形データ
	/// </summary>
	struct Triangle2DForGPU {
		InstanceData2D instance;
		Vector2 vertexOffsets[3];
	};

	/// <summary>
	/// 2D三角形
	/// </summary>
	class TriangleDrawer2D : public BaseDrawer2D {
	public:
		explicit TriangleDrawer2D(Graphics2DContext context);
		~TriangleDrawer2D() override;

		void Initialize() override;
		void EndFrame() override;

		void AddTriangle(const Triangle2DForGPU& data);

	private:
		Triangle2DForGPU* instanceData_ = nullptr;
	};

}
