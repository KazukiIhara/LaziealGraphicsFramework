#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <d3d12.h>

#include "Graphics/System/IGraphicsSubsystem.h"
#include "LGF/Graphics/PointLight.h"

namespace LGF {

	class DirectXCore;
	class PointLightResource;

	class PointLightSystem : public IGraphicsSubsystem {
	public:
		PointLightSystem(DirectXCore& directX);
		~PointLightSystem();

		bool Initialize() override;
		void BeginFrame() override;
		void EndFrame() override;
		void Finalize() override;

		void SetLight(const PointLight& light);
		void Bind(
			ID3D12GraphicsCommandList* commandList,
			uint32_t lightsRootParamIdx,
			uint32_t countRootParamIdx);

	private:
		std::vector<PointLight> lights_;
		std::unique_ptr<PointLightResource> resource_;
	};

}
