#pragma once

#include <cstdint>
#include <memory>

#include <d3d12.h>

#include "Graphics/System/IGraphicsSubsystem.h"
#include "LGF/Graphics/DirectionalLight.h"

namespace LGF {

	class DirectXCore;
	class DirectionalLightResource;

	class DirectionalLightSystem : public IGraphicsSubsystem {
	public:
		DirectionalLightSystem(DirectXCore& directX);
		~DirectionalLightSystem();

		bool Initialize() override;
		void BeginFrame() override;
		void EndFrame() override;
		void Finalize() override;

		void SetLight(const DirectionalLight& light);
		void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx);

	private:
		DirectionalLight light_;
		std::unique_ptr<DirectionalLightResource> resource_;
	};

}
