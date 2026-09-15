#pragma once

#include <cstdint>
#include <unordered_map>

#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "Graphics/System/IGraphicsSubsystem.h"

namespace LGF {

	class DirectXCore;
	class Pipeline;
	class PipelineManager;
	class Skybox;

	struct SkyboxForGPU {
		uint32_t textureIndex = UINT32_MAX;
		uint32_t irradianceTextureIndex = UINT32_MAX;
		uint32_t prefilteredTextureIndex = UINT32_MAX;
		uint32_t brdfLutTextureIndex = UINT32_MAX;
		float intensity = 0.0f;
		float rotation = 0.0f;
		uint32_t prefilteredMipCount = 0u;
		float padding = 0.0f;
	};
	static_assert(sizeof(SkyboxForGPU) == 32u);

	struct EnvironmentFilterForGPU {
		uint32_t sourceTextureIndex = UINT32_MAX;
		uint32_t outputTextureIndex = UINT32_MAX;
		uint32_t outputSize = 0u;
		uint32_t sampleCount = 0u;
		float roughness = 0.0f;
		float padding[3]{};
	};
	static_assert(sizeof(EnvironmentFilterForGPU) == 32u);

	class SkyboxSystem : public IGraphicsSubsystem {
	public:
		SkyboxSystem(DirectXCore& directX, PipelineManager& pipelineManager);
		~SkyboxSystem() override;

		bool Initialize() override;
		void BeginFrame() override;
		void EndFrame() override;
		void Finalize() override;

		void SetSkybox(const Skybox& skybox);
		void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) const;

	private:
		struct FilteredEnvironment {
			ComPtr<ID3D12Resource> irradiance;
			ComPtr<ID3D12Resource> prefiltered;
			uint32_t irradianceSrvIndex = UINT32_MAX;
			uint32_t prefilteredSrvIndex = UINT32_MAX;
			uint32_t prefilteredMipCount = 0u;
		};

		bool CreateBrdfLut();
		bool CreateFilteredEnvironment(uint32_t sourceTextureIndex, ID3D12Resource* sourceTexture);
		void DispatchFilter(
			Pipeline& pipeline,
			const EnvironmentFilterForGPU& constants,
			uint32_t constantSlot,
			uint32_t width,
			uint32_t height,
			uint32_t depth);

		DirectXCore& directX_;
		PipelineManager& pipelineManager_;
		ComPtr<ID3D12Resource> resource_ = nullptr;
		SkyboxForGPU* data_ = nullptr;
		SkyboxForGPU pendingData_{};
		ID3D12Resource* pendingSourceTexture_ = nullptr;

		ComPtr<ID3D12Resource> filterConstantsResource_ = nullptr;
		uint8_t* filterConstantsData_ = nullptr;
		ComPtr<ID3D12Resource> brdfLut_ = nullptr;
		uint32_t brdfLutSrvIndex_ = UINT32_MAX;
		std::unordered_map<uint32_t, FilteredEnvironment> filteredEnvironments_;
	};

}
