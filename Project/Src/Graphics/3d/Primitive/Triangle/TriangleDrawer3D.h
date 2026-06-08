#pragma once

#include "3d/BaseDrawer/BaseDrawer3D.h"
#include "3d/PrimitiveForGPU3D.h"

namespace LGF {

	class DirectXCore;

	/// <summary>
	/// 3D三角形描画クラス
	/// </summary>
	class TriangleDrawer3D :public BaseDrawer3D {
	public:
		TriangleDrawer3D(DirectXCore* directXCore);
		~TriangleDrawer3D()override = default;

		void Initialize()override;

		void ThrowDrawCommand()override;


	private:
		Triangle3DForGPU* instData_;


	};

}