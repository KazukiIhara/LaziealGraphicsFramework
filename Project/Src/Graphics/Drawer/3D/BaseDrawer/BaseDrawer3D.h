#pragma once

#include <cstdint>
#include <d3d12.h>

#include "ComPtr/ComPtr.h"

#include "Graphics/Drawer/BaseDrawer.h"

#include "LGF/Math/Vector4.h"
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
	};

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

	protected:
		Graphics3DContext context_;
	};

}