#pragma once

#include <memory>
#include <vector>

#include "Graphics/System/IGraphicsSubsystem.h"

#include "LGF/Math/MathTypes.h"

#include "Graphics/Graphics2DContext.h"

namespace LGF {

	class IDrawer;
	class TriangleDrawer2D;
	class RectDrawer2D;
	class CircleDrawer2D;
	class Triangle2D;
	class Rect2D;
	class Circle2D;
	class Line2D;

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

		// 三角形描画
		void AddTriangle(
			const Triangle2D& triangle,
			const Vector4& color
		);

		// 四角形描画
		void AddRect(
			const Rect2D& rect,
			const Vector4& color
		);

		// 円描画
		void AddCircle(
			const Circle2D& circle,
			const Vector4& color
		);

		// 線分描画
		void AddLine(
			const Line2D& line,
			const Vector4& color
		);

	private:
		// 参照
		Graphics2DContext context_;
		// 全更新用リスト
		std::vector<IDrawer*> drawerList_;

		// 描画クラス
		std::unique_ptr<TriangleDrawer2D> triangleDrawer_ = nullptr;
		std::unique_ptr<RectDrawer2D> rectDrawer_ = nullptr;
		std::unique_ptr<CircleDrawer2D> circleDrawer_ = nullptr;


	};
}
