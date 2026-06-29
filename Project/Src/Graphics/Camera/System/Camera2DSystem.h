#pragma once

#include <cstdint>
#include <memory>

#include <d3d12.h>

#include "LGF/Graphics/Camera2D.h"

#include "Graphics/System/IGraphicsSubsystem.h"

namespace LGF {

	class DirectXCore;
	class Camera2DResource;

	/// <summary>
	/// カメラ2Dシステム
	/// </summary>
	class Camera2DSystem : public IGraphicsSubsystem {
	public:
		Camera2DSystem(DirectXCore& directX);
		~Camera2DSystem();

		bool Initialize()override;
		void BeginFrame()override;
		void EndFrame()override;
		void Finalize()override;


		void SetCamera(const Camera2D& camera);

		void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx);

	private:
		// シーンで描画するカメラ
		Camera2D camera_;
		// GPU送るリソース
		std::unique_ptr<Camera2DResource> resource_;

	};


}