#pragma once

#include <memory>
#include <vector>

#include "Graphics/System/IGraphicsSubsystem.h"

#include "LGF/Math/MathTypes.h"

#include "Graphics/Graphics3DContext.h"

namespace LGF {

	class IDrawer;
	class TriangleDrawer3D;
	class Triangle3D;

	/// <summary>
	/// 全3D描画をするクラス
	/// </summary>
	class Renderer3D : public IGraphicsSubsystem {
	public:
		Renderer3D(Graphics3DContext context);
		~Renderer3D();

		bool Initialize()override;
		void BeginFrame()override;
		void EndFrame()override;
		void Finalize()override;

		// 各Drawerに描画コマンドを積む関数群

		// 三角形描画
		void AddTriangle(
			const Triangle3D& triangle,
			const Matrix4x4& worldMatrix,
			const Vector4& color
		);

	private:
		// 参照
		Graphics3DContext context_;
		// 全更新用リスト
		std::vector<IDrawer*> drawerList_;

		// 描画クラス
		std::unique_ptr<TriangleDrawer3D> triangleDrawer_;
	};

}