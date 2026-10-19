#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

#include "LGF/Math/Vector3.h"

namespace LGF {

	struct SphericalShellCap3DForGPU {
		InstanceData3D instance;
		float outerRadius = 2.0f;
		float innerRadius = 1.8f;
		float height = 1.0f;
		float padding = 0.0f;
	};

	class SphericalShellCapDrawer3D : public BaseDrawer3D {
	public:
		SphericalShellCapDrawer3D(Graphics3DContext context);
		~SphericalShellCapDrawer3D() override;

		void Initialize() override;
		void EndFrame() override;

		void AddSphericalShellCap(const SphericalShellCap3DForGPU& data);

	private:
		SphericalShellCap3DForGPU* instanceData_ = nullptr;
	};

}
