#pragma once

#include <memory>
#include <vector>

#include "Graphics/System/IGraphicsSubsystem.h"

namespace LGF {

	class IDrawer3D;
	class DirectXCore;
	class PipelineManager;
	class Camera3DSystem;

	class TriangleDrawer3D;

	/// <summary>
	/// 全3D描画をするクラス
	/// </summary>
	class Renderer3D : public IGraphicsSubsystem {
	public:
		struct Context {
			DirectXCore& directX;
			PipelineManager& pipelineMgr;
			Camera3DSystem& camera3DSys;
		};
	public:
		Renderer3D(Context context);
		~Renderer3D();

		bool Initialize()override;
		void BeginFrame()override;
		void EndFrame()override;
		void Finalize()override;

		// 各Drawerに描画コマンドを積む関数群


	private:
		// 参照
		Context context_;
		// 全更新用リスト
		std::vector<IDrawer3D*> drawerList_;


		// 描画クラス
		std::unique_ptr<TriangleDrawer3D> triangleDrawer3D_;
	};

}