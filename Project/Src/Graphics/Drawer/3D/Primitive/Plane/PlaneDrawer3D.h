#pragma once

#include <string>

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

#include "LGF/Math/Vector3.h"

namespace LGF {

	struct Plane3DForGPU {
		InstanceData3D instance;
		Vector3 vertexOffsets[4];
	};

	class PlaneDrawer3D :public BaseDrawer3D {
	public:
		PlaneDrawer3D(Graphics3DContext context, std::string pipelineName = "Plane3D");
		~PlaneDrawer3D()override;

		void Initialize()override;

		void EndFrame()override;

		void AddPlane(const Plane3DForGPU& data);

	private:
		std::string pipelineName_;
		Plane3DForGPU* instanceData_ = nullptr;

	};

}
