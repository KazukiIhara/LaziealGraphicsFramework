#pragma once

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"
#include "LGF/Math/Vector3.h"

#include <memory>
#include <vector>

namespace LGF {

	struct ParticleBillboardForGPU {
		Vector3 position{};
		float rotation{};
		Vector2 size{};
		uint32_t textureIndex = UINT32_MAX;
		float padding{};
		Vector4 color{};
	};
	static_assert(sizeof(ParticleBillboardForGPU) == 48u);

	// 描画容量はページ単位で拡張し、各ページのリソースをフレーム間で保持する。
	class ParticleDrawer3D : public BaseDrawer3D {
	public:
		explicit ParticleDrawer3D(Graphics3DContext context);
		void Initialize() override;
		void BeginFrame() override;
		void EndFrame() override;
		void AddParticle(const ParticleBillboardForGPU& data);

	private:
		static constexpr uint32_t kPageCapacity = 4096u;
		ParticleBillboardForGPU* instanceData_ = nullptr;
		std::unique_ptr<ParticleDrawer3D> nextPage_;
	};

}
