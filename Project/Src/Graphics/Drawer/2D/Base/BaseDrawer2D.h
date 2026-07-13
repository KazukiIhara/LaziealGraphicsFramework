#pragma once

#include <cstdint>
#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

#include "Graphics/Drawer/BaseDrawer.h"

#include "LGF/Math/Vector4.h"
#include "LGF/Math/Matrix4x4.h"

#include "Graphics/Graphics2DContext.h"

namespace LGF {

	/// <summary>
	/// 2Dインスタンシング描画用の構造体
	/// </summary>
	struct InstanceData2D {
		Vector4 baseColor;
	};

	/// <summary>
	/// 2D描画基底クラス
	/// </summary>
	class BaseDrawer2D :public BaseDrawer {
	public:
		BaseDrawer2D(Graphics2DContext context);
		virtual ~BaseDrawer2D()override;

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
		Graphics2DContext GetContext()const;

	private:
		Graphics2DContext context_;
	};



}
