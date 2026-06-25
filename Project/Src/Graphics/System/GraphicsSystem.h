#pragma once

#include <vector>
#include <memory>

#include "Core/Framework/System/IFrameworkSystem.h"

namespace LGF {

	class IGraphicsSubsystem;
	class DirectXCore;
	class PipelineManager;
	class Camera3DResource;
	class Camera3DSystem;
	class Renderer3D;
	class Renderer2D;

	class ImGuiSystem;

	/// <summary>
	/// グラフィックシステムクラス
	/// </summary>
	class GraphicsSystem :public IFrameworkSystem {
	public:
		GraphicsSystem();
		~GraphicsSystem()override;

		bool Initialize(FrameworkContext& context)override;
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
		// カメラ3Dシステム
		std::unique_ptr<Camera3DSystem> camera3DSystem_;

		// 3D描画クラス
		std::unique_ptr<Renderer3D> renderer3D_;

		// ImGuiシステムクラス
		std::unique_ptr<ImGuiSystem> imguiSystem_;
	};
}
