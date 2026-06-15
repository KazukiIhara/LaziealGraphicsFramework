#pragma once

#include <memory>
#include <vector>

namespace LGF {

	class TriangleDrawer3D;

	class IDrawer3D;
	class PipelineManager;

	/// <summary>
	/// 全3D描画をするクラス
	/// </summary>
	class Renderer3D {
	public:
		Renderer3D(PipelineManager& pipelineMgr);
		~Renderer3D();

		void Initialize();

		void EndFrame();

		// 各Drawerに描画コマンドを積む関数群


	private:
		// 描画クラス
		std::unique_ptr<TriangleDrawer3D> triangleDrawer3D_;

		// 全更新用リスト
		std::vector<IDrawer3D*> drawerList_;

		PipelineManager& pipelineMgr_;
	};

}