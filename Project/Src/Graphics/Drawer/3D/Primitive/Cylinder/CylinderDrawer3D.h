#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

namespace LGF {

	struct Cylinder3DForGPU {
		InstanceData3D instance;
		float radius = 0.5f;
		float height = 1.0f;
		float padding[2]{};
	};

	class CylinderDrawer3D :public BaseDrawer3D {
	public:
		CylinderDrawer3D(Graphics3DContext context);
		~CylinderDrawer3D()override;

		void Initialize()override;

		void EndFrame()override;

		void AddCylinder(const Cylinder3DForGPU& data);

	private:
		Cylinder3DForGPU* instanceData_ = nullptr;

	};

}
