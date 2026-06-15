#pragma once

#include <vector>
#include <memory>

#include "Framework/System/IFrameworkSystem.h"

namespace LGF {

	class IGraphicsSubsystem;
	class DirectXCore;
	class PipelineManager;
	class Renderer3D;
	class Renderer2D;

	/// <summary>
	/// グラフィックシステムクラス
	/// </summary>
	class GraphicsSystem :public IFrameworkSystem {
	public:
		GraphicsSystem();
		~GraphicsSystem()override;

		bool Initialize(FrameworkContext& context)override;
		bool Update(FrameworkContext& context)override;
		bool Finalize(FrameworkContext& context)override;

		void BeginFrame(FrameworkContext& context)override;
		void EndFrame(FrameworkContext& context)override;

	private:
		// 実行用リスト
		std::vector<IGraphicsSubsystem*> subsystems_;

		// DirectXSystem
		std::unique_ptr<DirectXCore> directX_;

		// パイプラインマネージャ
		std::unique_ptr<PipelineManager> pipelineManager_;

		// 3D描画クラス
		std::unique_ptr<Renderer3D> renderer3D_;

	};
}
