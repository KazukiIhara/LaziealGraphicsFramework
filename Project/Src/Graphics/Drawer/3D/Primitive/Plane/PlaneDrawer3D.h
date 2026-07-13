#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

#include "LGF/Math/Vector3.h"

namespace LGF {

	struct Plane3DForGPU {
		InstanceData3D instance;
		Vector3 vertexOffsets[4];
	};

	class PlaneDrawer3D :public BaseDrawer3D {
	public:
		PlaneDrawer3D(Graphics3DContext context);
		~PlaneDrawer3D()override;

		void Initialize()override;

		void EndFrame()override;

		void AddPlane(const Plane3DForGPU& data);

	private:
		Plane3DForGPU* instanceData_ = nullptr;

	};

}
