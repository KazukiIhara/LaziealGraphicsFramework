#pragma once

#include <cstdint>

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"
#include "LGF/Math/Vector3.h"

namespace LGF {

	struct MeshletBounds3DForGPU {
		InstanceData3D instance;
		float radius = 1.0f;
		Vector3 padding{};
	};
	static_assert(sizeof(MeshletBounds3DForGPU) == 192);

	class MeshletBoundsDrawer3D final : public BaseDrawer3D {
	public:
		explicit MeshletBoundsDrawer3D(Graphics3DContext context);
		~MeshletBoundsDrawer3D() override;

		void Initialize() override;
		void EndFrame() override;

		void AddBounds(const MeshletBounds3DForGPU& data);

	private:
		static constexpr uint32_t kMaxBoundsCount = 16384;

		MeshletBounds3DForGPU* instanceData_ = nullptr;
	};

}
