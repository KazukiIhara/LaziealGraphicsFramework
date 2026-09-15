#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include <d3d12.h>

#include "LGF/Graphics/Camera3D.h"

#include "Graphics/System/IGraphicsSubsystem.h"

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

		void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx);
		void BindCulling(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx);

	private:
		// シーンで使用するカメラ
		Camera3D camera_;
		std::optional<Camera3D> cullingCamera_;
		// GPUに送るカメラリソース
		std::unique_ptr<Camera3DResource> resource_;
		std::unique_ptr<Camera3DResource> cullingResource_;
	};
}
