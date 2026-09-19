#pragma once

#include <cstdint>
#include <d3d12.h>

#include "Graphics/DirectX/FrameContext/FrameContext.h"

#include "Graphics/DirectX/ComPtr/ComPtr.h"

#include "LGF/Math/Vector3.h"
#include "LGF/Math/Vector4.h"
#include "LGF/Math/Matrix4x4.h"

namespace LGF {

	class DirectXCore;
	class Camera3D;

	/// <summary>
	/// GPUに送るカメラデータ
	/// </summary>
	struct Camera3DForGPU {
		Matrix4x4 viewProjection;
		Matrix4x4 inverseViewProjection;
		Vector3 worldPosition;
		float padding;
		Vector4 frustumPlanes[6];
	};
	static_assert(sizeof(Camera3DForGPU) == 240);

	/// <summary>
	/// GPUに送る3Dカメラリソースを持つクラス
	/// </summary>
	class Camera3DResource {
	public:
		Camera3DResource(DirectXCore& directX);
		~Camera3DResource();

		void Initialize();
		void Update(const Camera3D& camera);

		void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx);

	private:
		// Camera用リソース
		FrameBufferedUploadBuffer resources_;

		DirectXCore& directX_;
	};
}
