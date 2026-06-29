#pragma once

#include <memory>
#include <vector>

#include "Graphics/System/IGraphicsSubsystem.h"

#include "LGF/Math/MathTypes.h"

#include "Graphics/Graphics2DContext.h"

namespace LGF {

	class IDrawer;
	class TriangleDrawer2D;
	class Triangle2D;

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
			const Matrix3x3& worldMatrix,
			const Vector4& color
		);

	private:
		// 参照
		Graphics2DContext context_;
		// 全更新用リスト
		std::vector<IDrawer*> drawerList_;

		// 描画クラス
		std::unique_ptr<TriangleDrawer2D> triangleDrawer_ = nullptr;


	};
}