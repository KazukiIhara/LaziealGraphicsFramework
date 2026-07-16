#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

#include "LGF/Math/Vector3.h"

namespace LGF {

	struct Sphere3DForGPU {
		InstanceData3D instance;
		float radius = 1.0f;
		Vector3 padding{};
	};

	class SphereDrawer3D :public BaseDrawer3D {
	public:
		SphereDrawer3D(Graphics3DContext context);
		~SphereDrawer3D()override;

		void Initialize()override;

		void EndFrame()override;

		void AddSphere(const Sphere3DForGPU& data);

	private:
		Sphere3DForGPU* instanceData_ = nullptr;

	};

}
