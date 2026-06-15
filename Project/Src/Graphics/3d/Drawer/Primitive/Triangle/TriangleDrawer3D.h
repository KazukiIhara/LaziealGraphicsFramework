#pragma once

#include "3d/Drawer/BaseDrawer/BaseDrawer3D.h"

#include "Math/Types/Vector4.h"

namespace LGF {

	class DirectXCore;

	/// <summary>
	/// GPUに送る3D三角形データ
	/// </summary>
	struct Triangle3DForGPU {
		InstanceData3D instance;
		Vector4 vertexOffsets[3];
	};

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