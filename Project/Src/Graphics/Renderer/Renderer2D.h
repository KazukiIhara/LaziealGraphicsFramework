#pragma once

#include <memory>
#include <vector>

#include "Graphics/System/IGraphicsSubsystem.h"

#include "LGF/Math/MathTypes.h"

#include "Graphics/Graphics2DContext.h"

namespace LGF {

	class IDrawer;

	/// <summary>
	/// 全2D描画をするクラス
	/// </summary>
	class Renderer2D :public IGraphicsSubsystem {
	public:
		Renderer2D(Graphics2DContext context);
		~Renderer2D();

		bool Initialize()override;
		void BeginFrame()override;
		void EndFrame()override;
		void Finalize()override;

		// 各Drawerに描画コマンドを積む関数群

	private:
		// 参照
		Graphics2DContext context_;
		// 全更新用リスト
		std::vector<IDrawer*> drawerList_;

		// 描画クラス

	};
}