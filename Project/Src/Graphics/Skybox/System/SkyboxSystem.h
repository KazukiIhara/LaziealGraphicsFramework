#pragma once

#include <cstdint>

#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "Graphics/System/IGraphicsSubsystem.h"

namespace LGF {

	class DirectXCore;
	class Skybox;

	struct SkyboxForGPU {
		uint32_t textureIndex = UINT32_MAX;
		float intensity = 0.0f;
		float rotation = 0.0f;
		float padding = 0.0f;
	};
	static_assert(sizeof(SkyboxForGPU) == 16u);

	class SkyboxSystem : public IGraphicsSubsystem {
	public:
		explicit SkyboxSystem(DirectXCore& directX);
		~SkyboxSystem() override;

		bool Initialize() override;
		void BeginFrame() override;
		void EndFrame() override;
		void Finalize() override;

		void SetSkybox(const Skybox& skybox);
		void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) const;

	private:
		DirectXCore& directX_;
		ComPtr<ID3D12Resource> resource_ = nullptr;
		SkyboxForGPU* data_ = nullptr;
		SkyboxForGPU pendingData_{};
	};

}
