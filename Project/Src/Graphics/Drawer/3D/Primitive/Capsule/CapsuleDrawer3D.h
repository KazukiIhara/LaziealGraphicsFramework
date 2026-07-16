#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

namespace LGF {

	struct Capsule3DForGPU {
		InstanceData3D instance;
		float radius = 0.25f;
		float height = 1.0f;
		float padding[2]{};
	};

	class CapsuleDrawer3D :public BaseDrawer3D {
	public:
		CapsuleDrawer3D(Graphics3DContext context);
		~CapsuleDrawer3D()override;

		void Initialize()override;

		void EndFrame()override;

		void AddCapsule(const Capsule3DForGPU& data);

	private:
		Capsule3DForGPU* instanceData_ = nullptr;

	};

}
