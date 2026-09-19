#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

#include <d3d12.h>

#include "Graphics/DirectX/FrameContext/FrameContext.h"

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "Graphics/Camera/Resource/Camera3DResource.h"
#include "Graphics/Shadow/ShadowCascadeCalculator.h"

namespace LGF {

	class DepthStencil;
	class DirectXCore;
	class Camera3D;
	class DirectionalLight;

	class ShadowMapSystem {
	public:
		explicit ShadowMapSystem(DirectXCore& directX);
		~ShadowMapSystem();

		bool Initialize();
		void Update(const Camera3D& camera, const DirectionalLight& light);
		void BeginShadowPass(uint32_t cascadeIndex);
		void EndShadowPass();
		void BindCamera(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIndex) const;
		void BindCascadeData(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIndex) const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetShadowMapHandleGPU() const;

	private:
		static constexpr uint32_t kShadowMapSize = 2048;
		static constexpr size_t kCameraConstantBufferStride = 256u;
		static_assert(sizeof(Camera3DForGPU) <= kCameraConstantBufferStride);
		static_assert(kCameraConstantBufferStride % D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT == 0u);

		struct ShadowCascadeDataForGPU {
			Matrix4x4 viewProjections[ShadowCascadeCalculator::kCascadeCount];
			Matrix4x4 view;
			Vector4 splitDepths;
			Vector4 settings;
		};
		static_assert(sizeof(ShadowCascadeDataForGPU) == 352u);

		DirectXCore& directX_;
		std::array<std::unique_ptr<DepthStencil>, ShadowCascadeCalculator::kCascadeCount> depths_;
		FrameBufferedUploadBuffer cameraResources_;
		FrameBufferedUploadBuffer cascadeDataResources_;
		ShadowCascadeCalculator::CascadeFrusta cascadeFrusta_{};
		uint32_t currentCascadeIndex_ = 0u;
	};

}
