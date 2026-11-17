#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <d3d12.h>

#include "Graphics/System/IGraphicsSubsystem.h"
#include "LGF/Graphics/Material.h"
#include "LGF/Graphics/ToonLighting.h"

namespace LGF {

	class DirectXCore;
	class ToonLightingResource;

	class ToonLightingSystem : public IGraphicsSubsystem {
	public:
		ToonLightingSystem(DirectXCore& directX);
		~ToonLightingSystem();

		bool Initialize() override;
		void BeginFrame() override;
		void EndFrame() override;
		void Finalize() override;

		void SetLighting(const ToonLighting& lighting);
		uint32_t RegisterMaterial(
			ShadingModel3D shadingModel,
			const ToonMaterial3D& toonLighting);
		void Bind(
			ID3D12GraphicsCommandList* commandList,
			uint32_t lightingRootParamIdx,
			uint32_t materialsRootParamIdx);

	private:
		ToonLighting lighting_;
		std::vector<ToonMaterial3D> materials_;
		std::unique_ptr<ToonLightingResource> resource_;
	};

}
