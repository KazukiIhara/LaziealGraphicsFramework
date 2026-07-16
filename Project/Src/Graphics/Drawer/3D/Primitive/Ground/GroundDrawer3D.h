#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

#include "LGF/Math/Vector3.h"

namespace LGF {

	struct Ground3DForGPU {
		InstanceData3D instance;
		Vector3 vertexOffsets[4];
	};

	class GroundDrawer3D :public BaseDrawer3D {
	public:
		GroundDrawer3D(Graphics3DContext context);
		~GroundDrawer3D()override;

		void Initialize()override;

		void EndFrame()override;

		void AddGround(const Ground3DForGPU& data);

	private:
		Ground3DForGPU* instanceData_ = nullptr;

	};

}
