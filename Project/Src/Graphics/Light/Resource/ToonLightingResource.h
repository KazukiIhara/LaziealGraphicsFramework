#pragma once

#include <array>
#include <cstdint>
#include <span>

#include <d3d12.h>

#include "Graphics/DirectX/FrameContext/FrameContext.h"
#include "LGF/Math/Vector3.h"

namespace LGF {

	class DirectXCore;
	class ToonLighting;
	struct ToonMaterial3D;

	/// <summary>
	/// GPUに送るToon Lightingデータ
	/// </summary>
	struct ToonLightingForGPU {
		Vector3 shadowColor;
		uint32_t enabled;
		Vector3 middleColor;
		float shadowThreshold;
		Vector3 lightColor;
		float lightThreshold;
		float smoothness;
		float padding[3];
	};
	static_assert(sizeof(ToonLightingForGPU) == 64u);

	struct ToonMaterialForGPU {
		Vector3 shadowColor;
		float shadowThreshold;
		Vector3 middleColor;
		float lightThreshold;
		Vector3 lightColor;
		float smoothness;
		float padding[4];
	};
	static_assert(sizeof(ToonMaterialForGPU) == 64u);

	class ToonLightingResource {
	public:
		static constexpr uint32_t MaxMaterialCount = 256u;

		ToonLightingResource(DirectXCore& directX);
		~ToonLightingResource();

		void Initialize();
		void Update(
			const ToonLighting& lighting,
			std::span<const ToonMaterial3D> materials);
		void Bind(
			ID3D12GraphicsCommandList* commandList,
			uint32_t lightingRootParamIdx,
			uint32_t materialsRootParamIdx);

	private:
		FrameBufferedUploadBuffer resources_;
		FrameBufferedUploadBuffer materialResources_;
		std::array<uint32_t, kFrameContextCount> materialSrvIndices_{};
		DirectXCore& directX_;
	};

}
