#pragma once

#include <cstdint>

#include "Graphics/Drawer/BaseDrawer.h"
#include "Graphics/Graphics2DContext.h"
#include "LGF/Math/Vector2.h"
#include "LGF/Math/Vector4.h"

namespace LGF {

	/// <summary>
	/// 2Dインスタンシング描画用の構造体
	/// </summary>
	struct InstanceData2D {
		Vector4 baseColor;
		Vector2 uvScale{ 1.0f, 1.0f };
		Vector2 uvOffset{};
		uint32_t textureIndex = UINT32_MAX;
	};
	static_assert(sizeof(InstanceData2D) == 36);

	/// <summary>
	/// 2D描画基底クラス
	/// </summary>
	class BaseDrawer2D : public BaseDrawer {
	public:
		explicit BaseDrawer2D(Graphics2DContext context);
		~BaseDrawer2D() override;

		/// <summary>
		/// 描画クラス固有の初期化処理
		/// </summary>
		void Initialize() override = 0;

		/// <summary>
		/// CPUで積んだコマンドをGPUに投げる
		/// </summary>
		void EndFrame() override = 0;

	protected:
		/// <summary>
		/// 参照メンバ構造体を取得
		/// </summary>
		Graphics2DContext GetContext() const;

	private:
		Graphics2DContext context_;
	};

}
