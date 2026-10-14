#pragma once

#include <cstdint>

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

	class PointLightResource {
	public:
		PointLightResource(DirectXCore& directX);
		~PointLightResource();

		void Initialize();
		void Update(const PointLight& light);
		void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx);

	private:
		FrameBufferedUploadBuffer resources_;
		DirectXCore& directX_;
	};

}
