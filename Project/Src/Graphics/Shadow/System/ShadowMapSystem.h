#pragma once

#include <cstdint>
#include <memory>

#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "Graphics/Camera/Resource/Camera3DResource.h"

namespace LGF {

	class DepthStencil;
	class DirectXCore;
	class DirectionalLight;

	class ShadowMapSystem {
	public:
		explicit ShadowMapSystem(DirectXCore& directX);
		~ShadowMapSystem();

		bool Initialize();
		void Update(const DirectionalLight& light);
		void BeginShadowPass();
		void EndShadowPass();
		void BindCamera(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIndex) const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetShadowMapHandleGPU() const;

	private:
		static constexpr uint32_t kShadowMapSize = 2048;
		DirectXCore& directX_;
		std::unique_ptr<DepthStencil> depth_;
		ComPtr<ID3D12Resource> cameraResource_;
		Camera3DForGPU* cameraData_ = nullptr;
	};

}
