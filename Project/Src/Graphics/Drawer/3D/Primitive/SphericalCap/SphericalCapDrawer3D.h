#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"

#include "LGF/Math/Vector2.h"

namespace LGF {

	struct SphericalCap3DForGPU {
		InstanceData3D instance;
		float baseRadius = 1.0f;
		float height = 0.5f;
		Vector2 padding{};
	};

	class SphericalCapDrawer3D : public BaseDrawer3D {
	public:
		SphericalCapDrawer3D(Graphics3DContext context);
		~SphericalCapDrawer3D() override;

		void Initialize() override;
		void EndFrame() override;

		void AddSphericalCap(const SphericalCap3DForGPU& data);

	private:
		SphericalCap3DForGPU* instanceData_ = nullptr;
	};

}
