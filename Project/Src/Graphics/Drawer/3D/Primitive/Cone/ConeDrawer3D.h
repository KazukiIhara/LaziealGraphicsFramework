#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

namespace LGF {

	struct Cone3DForGPU {
		InstanceData3D instance;
		float radius = 0.5f;
		float height = 1.0f;
		float padding[2]{};
	};

	class ConeDrawer3D :public BaseDrawer3D {
	public:
		ConeDrawer3D(Graphics3DContext context);
		~ConeDrawer3D()override;

		void Initialize()override;

		void EndFrame()override;

		void AddCone(const Cone3DForGPU& data);

	private:
		Cone3DForGPU* instanceData_ = nullptr;

	};

}
