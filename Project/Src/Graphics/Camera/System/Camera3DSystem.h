#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <array>

#include <d3d12.h>

#include "LGF/Graphics/Camera3D.h"

#include "Graphics/System/IGraphicsSubsystem.h"
#include "Graphics/RenderPass/RenderPassTypes.h"

namespace LGF {

	class DirectXCore;
	class Camera3DResource;

	/// <summary>
	/// カメラ3Dシステム
	/// </summary>
	class Camera3DSystem :public IGraphicsSubsystem {
	public:
		Camera3DSystem(DirectXCore& directX);
		~Camera3DSystem();

		bool Initialize()override;
		void BeginFrame()override;
		void EndFrame()override;
		void Finalize()override;


		void SetCamera(const Camera3D& camera);
		void SetCullingCamera(const Camera3D& camera);
		void SetShadowCamera(const Camera3D& camera);
		const Camera3D& GetShadowCamera() const;

		void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx);
		void BindCulling(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx);
		void PrepareRenderPass(uint32_t renderPassIndex, const Camera3D& camera);
		void SetActiveRenderPass(uint32_t renderPassIndex);
		uint32_t GetActiveRenderPass() const;
		const Camera3D& GetCamera() const;

	private:
		// シーンで使用するカメラ
		Camera3D camera_;
		std::optional<Camera3D> cullingCamera_;
		std::optional<Camera3D> shadowCamera_;
		// GPUに送るカメラリソース
		std::unique_ptr<Camera3DResource> resource_;
		std::unique_ptr<Camera3DResource> cullingResource_;
		std::array<std::unique_ptr<Camera3DResource>, kMaxRenderPassCount - 1u> renderPassResources_;
		uint32_t activeRenderPassIndex_ = 0u;
	};
}
