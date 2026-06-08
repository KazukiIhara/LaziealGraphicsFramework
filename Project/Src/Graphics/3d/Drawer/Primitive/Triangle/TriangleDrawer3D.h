#pragma once

#include "3d/Drawer/BaseDrawer/BaseDrawer3D.h"
#include "3d/PrimitiveForGPU3D.h"

namespace LGF {

	class DirectXCore;

	/// <summary>
	/// 3D三角形描画クラス
	/// </summary>
	class TriangleDrawer3D :public BaseDrawer3D {
	public:
		TriangleDrawer3D(DirectXCore* directXCore);
		~TriangleDrawer3D()override;

		void Initialize()override;

		void ThrowDrawCommand()override;

		void AddTriangle(const Triangle3DForGPU& data);

	private:
		Triangle3DForGPU* instData_ = nullptr;

	};

}