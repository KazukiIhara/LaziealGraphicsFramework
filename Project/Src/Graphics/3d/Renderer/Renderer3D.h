#pragma once

#include <memory>
#include <vector>

namespace LGF {

	class IDrawer3D;
	class PipelineManager;


	/// <summary>
	/// 全3D描画をするクラス
	/// </summary>
	class Renderer3D {
	public:
		Renderer3D(PipelineManager* pipelineMgr);
		~Renderer3D();

		void Initialize();

		void Render();

		// 各Drawerに描画コマンドを積む関数群


	private:
		// 全更新用リスト
		std::vector<IDrawer3D*> drawerList_;

		// 描画クラス群

		PipelineManager* pipelineMgr_;
	};

}