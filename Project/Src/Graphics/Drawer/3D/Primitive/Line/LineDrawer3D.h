#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

#include "LGF/Math/Vector3.h"

namespace LGF {

	struct Line3DForGPU {
		InstanceData3D instance;
		Vector3 start{};
		float padding0 = 0.0f;
		Vector3 end{};
		float padding1 = 0.0f;
	};

	class LineDrawer3D :public BaseDrawer3D {
	public:
		LineDrawer3D(Graphics3DContext context);
		~LineDrawer3D()override;

		void Initialize()override;

		void EndFrame()override;

		void AddLine(const Line3DForGPU& data);

	private:
		Line3DForGPU* instanceData_ = nullptr;

	};

}
