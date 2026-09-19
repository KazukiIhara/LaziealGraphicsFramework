#pragma once

#include <cstdint>

#include <d3d12.h>

#include "Graphics/DirectX/FrameContext/FrameContext.h"

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "LGF/Math/Vector3.h"

namespace LGF {

	class DirectXCore;
	class DirectionalLight;

	/// <summary>
	/// GPUに送る平行光源データ
	/// </summary>
	struct DirectionalLightForGPU {
		Vector3 direction;
		float intensity;
		Vector3 color;
		float padding;
	};

	class DirectionalLightResource {
	public:
		DirectionalLightResource(DirectXCore& directX);
		~DirectionalLightResource();

		void Initialize();
		void Update(const DirectionalLight& light);
		void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx);

	private:
		FrameBufferedUploadBuffer resources_;
		DirectXCore& directX_;
	};

}
