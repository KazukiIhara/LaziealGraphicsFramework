#pragma once

#include <cstdint>
#include <d3d12.h>

#include "Graphics/DirectX/FrameContext/FrameContext.h"

#include "Graphics/DirectX/ComPtr/ComPtr.h"

#include "LGF/Math/Vector2.h"
#include "LGF/Math/Matrix4x4.h"

namespace LGF {

	class DirectXCore;
	class Camera2D;

	/// <summary>
	/// GPUに送るカメラデータ
	/// </summary>
	struct Camera2DForGPU {
		Matrix4x4 viewProjection;
	};

	/// <summary>
	/// GPUに送る2Dカメラリソースを持つクラス
	/// </summary>
	class Camera2DResource {
	public:
		Camera2DResource(DirectXCore& directX);
		~Camera2DResource();

		void Initialize();
		void Update(const Camera2D& camera);

		void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx);

	private:
		// Camera用リソース
		FrameBufferedUploadBuffer resources_;

		DirectXCore& directX_;
	};
}
