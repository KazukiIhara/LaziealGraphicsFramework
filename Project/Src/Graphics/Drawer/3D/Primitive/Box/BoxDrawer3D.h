#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

#include "LGF/Math/Vector3.h"

namespace LGF {

	struct Box3DForGPU {
		InstanceData3D instance;
		Vector3 halfSize{};
		float padding = 0.0f;
	};

	class BoxDrawer3D :public BaseDrawer3D {
	public:
		BoxDrawer3D(Graphics3DContext context);
		~BoxDrawer3D()override;

		void Initialize()override;

		void EndFrame()override;

		void AddBox(const Box3DForGPU& data);

	private:
		Box3DForGPU* instanceData_ = nullptr;

	};

}
