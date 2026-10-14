#pragma once

#include <array>
#include <cstdint>
#include <span>

#include <d3d12.h>

#include "Graphics/DirectX/FrameContext/FrameContext.h"

#include "LGF/Math/Vector3.h"

namespace LGF {

	class DirectXCore;
	class PointLight;

	/// <summary>
	/// GPUに送る点光源データ
	/// </summary>
	struct PointLightForGPU {
		Vector3 position;
		float intensity;
		Vector3 color;
		float range;
	};
	static_assert(sizeof(PointLightForGPU) == 32u);

	class PointLightResource {
	public:
		PointLightResource(DirectXCore& directX);
		~PointLightResource();

		void Initialize();
		void Update(std::span<const PointLight> lights);
		void Bind(
			ID3D12GraphicsCommandList* commandList,
			uint32_t lightsRootParamIdx,
			uint32_t countRootParamIdx);

	private:
		struct PointLightCountForGPU {
			uint32_t count;
			float padding[3];
		};
		static_assert(sizeof(PointLightCountForGPU) == 16u);

		FrameBufferedUploadBuffer lightResources_;
		FrameBufferedUploadBuffer countResources_;
		std::array<uint32_t, kFrameContextCount> lightSrvIndices_{};
		DirectXCore& directX_;
	};

}
