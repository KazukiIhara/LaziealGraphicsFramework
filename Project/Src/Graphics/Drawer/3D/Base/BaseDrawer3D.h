#pragma once

#include <cstdint>
#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

#include "Graphics/Drawer/BaseDrawer.h"

#include "LGF/Math/Vector4.h"
#include "LGF/Math/Vector2.h"
#include "LGF/Math/Matrix4x4.h"

#include "Graphics/Graphics3DContext.h"

namespace LGF {

	/// <summary>
	/// 3Dインスタンシング描画用の構造体
	/// </summary>
	struct InstanceData3D {
		Matrix4x4 worldMatrix;
		Matrix4x4 worldInverseTranspose;
		Vector4 baseColor;
		Vector2 uvScale = { 1.0f, 1.0f };
		Vector2 uvOffset{};
		uint32_t textureIndex = UINT32_MAX;
		uint32_t normalTextureIndex = UINT32_MAX;
		float normalScale = 1.0f;
		uint32_t padding = 0;
	};
	static_assert(sizeof(InstanceData3D) == 176);

	/// <summary>
	/// 3D描画基底クラス
	/// </summary>
	class BaseDrawer3D :public BaseDrawer {
	public:
		BaseDrawer3D(Graphics3DContext context);
		virtual ~BaseDrawer3D()override;

		/// <summary>
		/// 描画クラス固有の初期化処理
		/// </summary>
		virtual void Initialize()override = 0;

		/// <summary>
		/// CPUで積んだコマンドをGPUに投げる
		/// </summary>
		virtual void EndFrame()override = 0;

	protected:
		/// <summary>
		/// 参照メンバ構造体を取得
		/// </summary>
		Graphics3DContext GetContext() const;

	private:
		Graphics3DContext context_;
	};

}
