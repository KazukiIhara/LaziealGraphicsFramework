#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace LGF {

	class BaseDrawer3D;
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

	private:
		std::unordered_map<std::string, std::unique_ptr<BaseDrawer3D>> drawerMap_;
		std::vector<BaseDrawer3D*> drawerList_;

		PipelineManager* pipelineMgr_;
	};

}