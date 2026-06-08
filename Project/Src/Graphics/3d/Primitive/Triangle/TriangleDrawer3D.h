#pragma once

#include "3d/BaseDrawer/BaseDrawer3D.h"
#include "3d/PrimitiveForGPU3D.h"
#include "LGF/Graphics/Primitive.h"
#include "LGF/Graphics/Material.h"

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

		void AddTriangle(const Graphics3D::Triangle& tri, const Graphics3D::Material& mat);

	private:
		Triangle3DForGPU* instData_ = nullptr;

	};

}